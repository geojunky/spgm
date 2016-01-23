/*
 * =====================================================================================
 * Scalable PaleoGeomorphology Model (SPGM)
 *
 * Copyright (C) 2014 Rakib Hassan (rakib.hassan@sydney.edu.au)
 *
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
 * Place, Suite 330, Boston, MA 02111-1307 USA
 * ===================================================================================== 
 */
/*
 * =====================================================================================
 *
 *       Filename:  FluvialErosion.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:41:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <SurfaceTopology.hh>
#include <FluvialErosion.hh>
#include <Model.hh>
#include <ScalarField.hh>
#include <Timer.hh>

namespace src{ namespace model {

    /*
     * =====================================================================================
     *        Class:  NewtonRaphson
     *  Description:  Implementation of Newton-Raphson root-finder adapted for solving the 
     *                non-linear transport equation as embodied by the stream-power law.
     * =====================================================================================
     */
    class NewtonRaphson
    {
        /*-----------------------------------------------------------------------------
         * This 'safe' Newton-Raphson scheme has been adapted from Numerical Recipes in
         * C++ (2nd Edition, page 370).
         * The parameters to this class are as follows:
         * ht1si    : Height of current node, in stack order, at the next time-step
         * htsi     : Height of current node, in stack order, at this time-step
         * ht1rsi   : Height of receiver node of current node, in stack order, at the
         *            next time-step
         * C        : Constant related to catchment-area raised to some power
         * d        : Euclidean distance between current node and its receiver
         * n        : Power to which local-slope is raised to
         * acc      : Accuracy the solver must attain
         *-----------------------------------------------------------------------------*/
        public:
            NewtonRaphson(double ht1si, double htsi, double ht1rsi, double C, double d, double n, double acc)
            :m_ht1si(ht1si),
            m_htsi(htsi),
            m_ht1rsi(ht1rsi),
            m_C(C),
            m_d(d),
            m_n(n),
            m_acc(acc)
            {
            }
            
            double Solve(int *itercount)
            {
                const int MAXIT=100;
                int j;
                double df,dx,dxold,f,fh,fl,temp,xh,xl,rts;
                
                double x1 = m_ht1si; /* Upper bound - i.e height of current node */
                double x2 = m_ht1rsi; /* Lower bound - i.e height of receiver node */
                
                Func(x1,&fl,&df);
                Func(x2,&fh,&df);
                if ((fl > 0.0 && fh > 0.0) || (fl < 0.0 && fh < 0.0))
                {
                    cerr << "Error: Root must be bracketed in Newton-Raphson solver" << endl;
                    exit(EXIT_FAILURE);
                }
                if (fl == 0.0) return x1;
                if (fh == 0.0) return x2;
                if (fl < 0.0)
                {
                    xl=x1;
                    xh=x2;
                }
                else
                {
                    xh=x1;
                    xl=x2;
                }
                rts=0.5*(x1+x2);
                dxold=fabs(x2-x1);
                dx=dxold;
                Func(rts,&f,&df);
                *itercount = 0;
                for (j=0;j<MAXIT;j++, ++(*itercount))
                {
                    if ((((rts-xh)*df-f)*((rts-xl)*df-f) > 0.0)
                        || (fabs(2.0*f) > fabs(dxold*df)))
                    {
                        dxold=dx;
                        dx=0.5*(xh-xl);
                        rts=xl+dx;
                        if (xl == rts) return rts;
                    }
                    else
                    {
                        dxold=dx;
                        dx=f/df;
                        temp=rts;
                        rts -= dx;
                        if (temp == rts) return rts;
                    }
                    if (fabs(dx) < m_acc) return rts;
                    Func(rts,&f,&df);
                    if (f < 0.0)
                        xl=rts;
                    else
                        xh=rts;
                }
                cerr << "Error: Maximum number of iterations exceeded in rtsafe" << endl;
                exit(EXIT_FAILURE);
                return 0.0;
            }
            
        private:
            /*
             *--------------------------------------------------------------------------------------
             *       Class:  NewtonRaphson
             *      Method:  NewtonRaphson :: Func
             * Description:  See eq. 24, Braun et al. (2013) for more details.
             *--------------------------------------------------------------------------------------
             */
            void Func(double x, double *f, double *df)
            {
                *f  = x - m_htsi + m_C*pow((x-m_ht1rsi)/m_d, m_n);
                *df = (1+m_n*m_C/m_d*pow((x-m_ht1rsi)/m_d, m_n-1));
            }
            
            double m_ht1si;
            double m_htsi;
            double m_ht1rsi;
            double m_C;
            double m_d;
            double m_n;
            double m_acc;
    };
    
    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosion
     *      Method:  FluvialErosion :: FluvialErosion
     * Description:  Constructor reads in parameters from config specific to this class.
     *--------------------------------------------------------------------------------------
     */
    FluvialErosion::FluvialErosion(const Model *m, Config *c)
    :Process(m, c)
    {
        /*-----------------------------------------------------------------------------
         * Read parameters
         *-----------------------------------------------------------------------------*/
        m_streamPowerM      = m_config->PDouble("m");
        m_streamPowerN      = m_config->PDouble("n");
        m_solverTolerance   = m_config->PDouble("tolerance");
        m_Kf                = m_config->PDouble("erosionCoefficient");

        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        int len = st->GetNMeshPoints();
        ScalarField<float> *catchmentArea = new ScalarField<float>("catchmentArea", len);
        
        m_model->AddField(catchmentArea);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosion
     *      Method:  FluvialErosion :: ~FluvialErosion
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    FluvialErosion::~FluvialErosion()
    {
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosion
     *      Method:  FluvialErosion :: ComputeCatchmentArea
     * Description:  The catchment-area upstream of each node is computed by traversing the
     *               flow-network in reverse stack-order.
     *--------------------------------------------------------------------------------------
     */
    void FluvialErosion::ComputeCatchmentArea()
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        const int *hull = st->GetHull();
        ScalarField<float> *catchmentArea = static_cast< ScalarField<float>* > (m_model->GetField("catchmentArea"));

        const float *surfaceArea = st->GetVoronoiCellAreas();
        int len = st->GetNMeshPoints();
        
        float averageCellArea = st->GetAverageCellArea();
        for(int i=0; i<len; i++)
        {
            if(!hull[i]) (*catchmentArea)(i) = surfaceArea[i];
            else (*catchmentArea)(i) = averageCellArea;
        }

        /*-----------------------------------------------------------------------------
         * Compute total catchment area upstream of a node by traversing the stack in
         * reverse - i.e. from atop hills
         *-----------------------------------------------------------------------------*/
        for(int i=0; i<len; i++)
        {
            int index = len - i - 1;
            if(st->S(index) == INVALID) continue;
            if(st->SR( st->S(index) ) != st->S(index))
                (*catchmentArea)( st->SR(st->S(index)) ) += (*catchmentArea)( st->S(index) );
        }

#ifdef DEBUG
        cout << endl << "Catchment Area Upstream: " << endl;
        cout <<         "------------------------ " << endl;
        for(int i=0; i<len; i++)
        {
            if(st->S(i) != INVALID)
                cout << st->S(i) << " " << (*catchmentArea)( st->S(i) ) << endl;
            else
                cout << st->S(i) << " " << 0. << endl;
        }
#endif
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosion
     *      Method:  FluvialErosion :: SolveStreamPowerEquation
     * Description:  Solves the stream-power law. See section 4, Braun et al. (2013) for 
     *               more details.
     *--------------------------------------------------------------------------------------
     */
    void FluvialErosion::SolveStreamPowerEquation()
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        ScalarField<float> *catchmentArea = static_cast< ScalarField<float>* > (m_model->GetField("catchmentArea"));

        int len = st->GetNMeshPoints();
        ScalarField<float> Z("z", len);
        for(int i=0; i<len; i++) Z(i) = st->Z(i);
        vector<int> solved(len);
        vector<int> catchmentTags;
        float dt = m_model->GetDt();
        
        /*-----------------------------------------------------------------------------
         * Get unique catchment tags
         *-----------------------------------------------------------------------------*/
        for(SurfaceTopology::CatchmentIterator cit = st->CatchmentsBegin(); cit != st->CatchmentsEnd(); cit++)
        {
            catchmentTags.push_back(*cit);
        }

        Timer tBegin; 
#pragma omp parallel for
        for(unsigned int ci=0; ci<catchmentTags.size(); ci++)
        {
            int c = catchmentTags[ci];

            for(int i=0; i<len; i++)
            {
                double xdiff, ydiff, d, ht1si, htsi, ht1rsi, oldht1si, C;
                int si = st->S(i);
                int rsi = st->R(si);
                
                if (si == INVALID)
                {
                    solved[i] = 1;
                    continue;
                }
                
                if(st->C(si) != c) continue;

                /*-----------------------------------------------------------------------------
                 * For outlet nodes copy height and move on
                 *-----------------------------------------------------------------------------*/
                if (si == rsi)
                {
                    solved[si] = 1;
                    continue;
                }

                ht1si = htsi = oldht1si = st->Z(si);
                ht1rsi= Z( rsi );
                C = m_Kf * pow((*catchmentArea)(si), m_streamPowerM) * dt;
                xdiff = fabs(st->X(si) - st->X(rsi));
                ydiff = fabs(st->Y(si) - st->Y(rsi));
                d = sqrt(xdiff*xdiff + ydiff*ydiff);

                /*-----------------------------------------------------------------------------
                 * For n==1 the stream power function can be solved explicitly
                 *-----------------------------------------------------------------------------*/
                if(m_streamPowerN == 1)
                {
                    ht1si = (htsi + ht1rsi*C/d) / (1+C/d);
                }
                else
                {
                    int itercount = 0;
                    NewtonRaphson nr(ht1si, htsi, ht1rsi, C, d, m_streamPowerN, m_solverTolerance);

                    ht1si = nr.Solve(&itercount);
                }
                
                if((!st->B(si)) && (si != rsi) )
                {
                    solved[si] = 1;
                    Z(si) = ht1si;
                }
            }
        }
        Timer tEnd; 
        printf("aa %e %d \n", Timer::Elapsed(tBegin, tEnd), catchmentTags.size());

//#define DEBUG
#ifdef DEBUG
        cout << endl << "Nodal Heights (Si Zi Zi_old): " << endl;
        cout <<         "----------------------------- " << endl;
        for(int i=0; i<len; i++)
            if (st->S(i) != INVALID)
                printf("%d %9.9lf %9.9lf\n", st->S(i), Z(st->S(i)), st->Z(st->S(i)));
            else
                printf("%d %9.9lf %9.9lf\n", st->S(i), 0., 0.);
#endif
        /*-----------------------------------------------------------------------------
         * Update height-field in SurfaceTopology
         *-----------------------------------------------------------------------------*/
        for(int i=0; i<len; i++)
        {
            if(solved[i]) Z(i) -= st->Z(i); /* Compute net changes */
            else Z(i) = 0;
        }
        
        st->UpdateZ(&Z);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosion
     *      Method:  FluvialErosion :: Execute
     * Description:  Compute fluvial-erosion
     *--------------------------------------------------------------------------------------
     */
    void FluvialErosion::Execute()
    {
        if(m_model->GetTimeStep() % m_frequency) return;

        ComputeCatchmentArea();
        SolveStreamPowerEquation();
    }
}}

