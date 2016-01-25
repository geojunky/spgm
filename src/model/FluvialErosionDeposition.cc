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
 *       Filename:  FluvialErosionDeposition.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  17/04/15 14:41:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <SurfaceTopology.hh>
#include <FluvialErosionDeposition.hh>
#include <Model.hh>
#include <ScalarField.hh>
#include <Timer.hh>

namespace src{ namespace model {

    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosionDeposition
     *      Method:  FluvialErosionDeposition :: FluvialErosionDeposition
     * Description:  Constructor reads in parameters from config specific to this class.
     *--------------------------------------------------------------------------------------
     */
    FluvialErosionDeposition::FluvialErosionDeposition(const Model *m, Config *c)
    :Process(m, c)
    {
        /*-----------------------------------------------------------------------------
         * Read parameters
         *-----------------------------------------------------------------------------*/
        m_Kf                = m_config->PDouble("erosionCoefficient");
        m_Lea               = m_config->PDouble("alluvialErosionLengthScale");
        m_Leb               = m_config->PDouble("bedrockErosionLengthScale");
        m_streamPowerM      = m_config->PDouble("m");
        m_streamPowerN      = m_config->PDouble("n");

        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        int len = st->GetNMeshPoints();
        ScalarField<float> *sediment         = new ScalarField<float>("sediment", len);
        ScalarField<float> *sedimentHistory  = new ScalarField<float>("sedimentHistory", len);
        ScalarField<float> *discharge        = new ScalarField<float>("discharge", len);
        
        m_model->AddField(sediment);
        m_model->AddField(sedimentHistory);
        m_model->AddField(discharge);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosionDeposition
     *      Method:  FluvialErosionDeposition :: ~FluvialErosionDeposition
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    FluvialErosionDeposition::~FluvialErosionDeposition()
    {
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosionDeposition
     *      Method:  FluvialErosionDeposition :: ComputeDischarge
     * Description:  Compute discharge at every node.
     *--------------------------------------------------------------------------------------
     */
    void FluvialErosionDeposition::ComputeDischarge()
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        ScalarField<float> *discharge = static_cast< ScalarField<float>* > (m_model->GetField("discharge"));
        ScalarField<float> *precipitation = static_cast< ScalarField<float>* > (m_model->GetField("precipitation"));

        int len = st->GetNMeshPoints();
        for(int i=0; i<len; i++)
        {
            (*discharge)(i) = m_Kf*(*precipitation)(i);
        }

        /*-----------------------------------------------------------------------------
         * Compute total discharge at a node by traversing the stack in reverse - i.e.
         * from atop hills
         *-----------------------------------------------------------------------------*/
        for(int i=0; i<len; i++)
        {
            int index = len - i - 1;
            if(st->S(index) == INVALID) continue;
            if(st->SR( st->S(index) ) != st->S(index))
                (*discharge)( st->SR(st->S(index)) ) += (*discharge)( st->S(index) );
            
            //if(st->R( st->S(index) ) != st->S(index))
            //    (*discharge)( st->R(st->S(index)) ) += (*discharge)( st->S(index) );
        }
        
#ifdef DEBUG
        cout << endl << "Nodal Discharge: " << endl;
        cout <<         "---------------- " << endl;
        for(int i=0; i<len; i++)
        {
            if(st->S(i) != INVALID)
                cout << st->S(i) << " " << (*discharge)( st->S(i) ) << endl;
            else
                cout << st->S(i) << " " << 0. << endl;
        }
#endif
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  FluvialErosionDeposition
     *      Method:  FluvialErosionDeposition :: SolveStreamPowerEquation
     * Description:  Solves the stream-power law. See section 4, Braun et al. (2013) for 
     *               more details.
     *--------------------------------------------------------------------------------------
     */
    void FluvialErosionDeposition::SolveStreamPowerEquation()
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        ScalarField<float> *discharge = static_cast< ScalarField<float>* > (m_model->GetField("discharge"));
        ScalarField<float> *sediment = static_cast< ScalarField<float>* > (m_model->GetField("sediment"));
        ScalarField<float> *sedimentHistory = static_cast< ScalarField<float>* > (m_model->GetField("sedimentHistory"));

        int len = st->GetNMeshPoints();
        const float *vareas = st->GetVoronoiCellAreas();
        vector<int> solved(len);
        vector<int> catchmentTags;
        
        ScalarField<float> Z("z", len);
        for(int i=0; i<len; i++) 
        {
            Z(i) = st->Z(i);
            (*sediment)(i) = 0;
        }
        
        /*-----------------------------------------------------------------------------
         * Get unique catchment tags
         *-----------------------------------------------------------------------------*/
        for(SurfaceTopology::CatchmentIterator cit = st->CatchmentsBegin(); cit != st->CatchmentsEnd(); cit++)
        {
            catchmentTags.push_back(*cit);
        }
        vector<int> clippedCount(catchmentTags.size()+1);

#pragma omp parallel for
        for(unsigned int ci=0; ci<catchmentTags.size(); ci++)
        {
            int c = catchmentTags[ci];

            for(int index=0; index<len; index++)
            {
                int i = len - index - 1;
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
                if ((si == rsi))
                {
                    solved[si] = 1;
                    continue;
                }

                /*-----------------------------------------------------------------------------
                 * Compute slope and Qe 
                 *-----------------------------------------------------------------------------*/
                float dx    = sqrt(pow(st->X(si) - st->X(rsi), float(2.)) + pow(st->Y(si) - st->Y(rsi), float(2.)));
                float dh    = st->Z(si) - st->Z(rsi);
                float slope = dh/dx;
                float Qe    = m_Kf * pow(slope, m_streamPowerN) * pow((*discharge)(si), m_streamPowerM);

erosionDeposition:
                if((*sediment)(si) < Qe)
                {
                    /*-----------------------------------------------------------------------------
                     * Erosion taking place
                     *-----------------------------------------------------------------------------*/
                    
                    float QeDeficit = 0;
                    float fac = 0.;
                    if((*sedimentHistory)(si) > 0)
                        fac = dx/m_Lea;
                    else
                        fac = dx/m_Leb;
                    
                    float dz = ((*sediment)(si) - Qe) / vareas[si] * fac;
                    
                    /*-----------------------------------------------------------------------------
                     * Check if sediment-thickness < dz, to ensure that the correct 'fac'
                     * is used for sediments and bedrock.
                     *-----------------------------------------------------------------------------*/
                    bool processAgain = false;
                    if(((*sedimentHistory)(si) > 0) && (fabs(dz) > (*sedimentHistory)(si)))
                    {
                        //printf("%5.5e, %5.5e\n", (*sedimentHistory)(si), fabs(dz));

                        dz = -(*sedimentHistory)(si);
                        processAgain = true;
                    }

                    /*-----------------------------------------------------------------------------
                     * Sanity check: It is assumed that erosion of material at a cell cannot
                     * lower the corresponding node below its reveiver node. In other words,
                     * a clip is applied.
                     *-----------------------------------------------------------------------------*/                    
                    if(!st->B(si)  && (si != rsi)) 
                    {
                        if((st->Z(si) + dz) < st->Z(rsi))
                        {
                            float diff = st->Z(rsi) - st->Z(si);
                            QeDeficit  = (dz - diff)*vareas[si];
                            dz         = diff; 
                            clippedCount[ci]++;
                        }
                    }                            

                    (*sedimentHistory)(si) += dz;
                    Z(si) += dz;
                    (*sediment)(si) += fabs((*sediment)(si) - Qe - QeDeficit) * fac;

                    if(processAgain) goto erosionDeposition;
                }
                else if((*sediment)(si) > Qe)
                {
                    /*-----------------------------------------------------------------------------
                     * Deposition taking place 
                     *-----------------------------------------------------------------------------*/
                    
                    float dz = ((*sediment)(si) - Qe) / vareas[si];
                    float QeExcess = 0;

                    /*-----------------------------------------------------------------------------
                     * Sanity check: It is assumed that deposition of sediments at a cell cannot
                     * elevate the corresponding node above its lowest donor node. In other words,
                     * a clip is applied.
                     *-----------------------------------------------------------------------------*/
                    if(!st->B(si)  && (si != rsi)) 
                    {
                        int dn = st->Dn(si);
                        const int *d = st->D(si);

                        for(int j=0; j<dn; j++)
                        {
                            if((st->Z(si) + dz) > st->Z(d[j]))
                            {
                                /* Get lowest donor idx*/
                                float minDiff = 1e20;
                                for(int k=0; k<dn; k++)
                                {
                                    float diff = st->Z(d[k]) - st->Z(si);
                                    if(minDiff > diff)
                                    {
                                        minDiff = diff;
                                    }
                                }

                                QeExcess    = (dz - minDiff)*vareas[si];
                                dz          = minDiff;

                                clippedCount[ci]++;
                            }
                        }
                    }            
                    
                    (*sedimentHistory)(si) += dz;
                    Z(si) += dz;
                    (*sediment)(si) = Qe + QeExcess;
                }

                if(!st->B(si)  && (si != rsi))
                {
                    solved[si] = 1;
                    
                    (*sediment)(rsi) += (*sediment)(si);
                }
            }
        }

        /*-----------------------------------------------------------------------------
         * Report stats 
         *-----------------------------------------------------------------------------*/
#ifdef DEBUG        
        int clippedCountTotal = 0;
        for(unsigned int ci=0; ci<catchmentTags.size(); ci++) clippedCountTotal+=clippedCount[ci];
        if(clippedCountTotal)
        {
            printf("\tCarrying capacity of %2.2f %% of the nodes was altered - consider reducing time-step..\n",
                    float(clippedCountTotal)/float(len)*100.);
        }
#endif

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
     *       Class:  FluvialErosionDeposition
     *      Method:  FluvialErosionDeposition :: Execute
     * Description:  Compute fluvial-erosion
     *--------------------------------------------------------------------------------------
     */
    void FluvialErosionDeposition::Execute()
    {
        if(m_model->GetTimeStep() % m_frequency) return;

        ComputeDischarge();
        SolveStreamPowerEquation();
    }
}}

