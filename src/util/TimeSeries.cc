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
 *       Filename:  TimeSeries.cc
 *
 *    Description:  TimeSeries class
 *
 *        Version:  1.0
 *        Created:  28/12/15 15:47:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <TimeSeries.hh>
#include <Model.hh>
#include <SurfaceTopology.hh>
#include <Log.hh>

namespace src { namespace util {
    using namespace std;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  TimeSeries
     *      Method:  TimeSeries :: TimeSeries
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    TimeSeries::TimeSeries(const Model *m, Config *c, string paramName):
    m_model(m),
    m_config(c),
    m_paramName(paramName)
    {
        m_isSingleValued    = false;
        m_isTimeSeries      = false;
        m_isFieldTimeSeries = false;
        /*-----------------------------------------------------------------------------
         * Read parameters
         *-----------------------------------------------------------------------------*/
        m_value = m_config->PDouble(m_paramName);
        m_file  = m_config->PString(m_paramName);
        
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        int nn = st->GetNMeshPoints();

        /*-----------------------------------------------------------------------------
         * Check if single-valued
         *-----------------------------------------------------------------------------*/
        double dummy;
        m_isSingleValued = (sscanf(m_file.c_str(), "%lf", &dummy)==1);

        if(!m_isSingleValued)
        {
            AscertainTimeSeriesType();

            if(m_times.size()<2)
            {
                LogError(cout << "File: " << m_file << " must have atleast 2 entries.." << endl);
                exit(EXIT_FAILURE);
            }
        }
        
        m_idxLo = 0;
        m_fieldValueLo.resize(nn);
        m_fieldValueHi.resize(nn);
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  TimeSeries
     *      Method:  TimeSeries :: ascertainTimeSeriesType
     * Description:  Check if input corresponds to a 1D or a Field time-series
     *--------------------------------------------------------------------------------------
     */
    void TimeSeries::AscertainTimeSeriesType()
    {
        /*-----------------------------------------------------------------------------
         * Initial validation
         *-----------------------------------------------------------------------------*/
        {
            FILE *f = fopen(m_file.c_str(), "r");
            char buffer[1024] = {0};

            if(f==NULL) 
            {
                LogError(cout << "Could not open file: " << m_file << endl);
                exit(EXIT_FAILURE);
            }

            int lineCount = 0;
            int result1D=0, result2D=0;
            while(fgets(buffer, sizeof(buffer), f) != NULL)
            {
                float t, dummy;
                char fn[2048] = {0};

                result1D += sscanf(buffer, "%f %f", &t, &dummy);
                result2D += sscanf(buffer, "%f %s", &t, fn);
                
                lineCount++;
            }
            
            if(result1D == lineCount*2)
                m_isTimeSeries = true;
            else if(result2D == lineCount*2)
                m_isFieldTimeSeries = true;
            else
            {
                LogError(cout << "Error in time-series file format.." << endl);
                exit(EXIT_FAILURE);
            }
            
            fclose(f);
        }
        
        /*-----------------------------------------------------------------------------
         * Read pairs
         *-----------------------------------------------------------------------------*/
        {
            FILE *f = fopen(m_file.c_str(), "r");
            char buffer[1024] = {0};
            
            if(f==NULL) 
            {
                LogError(cout << "Could not open file: " << m_file << endl);
                exit(EXIT_FAILURE);
            }

            while(fgets(buffer, sizeof(buffer), f) != NULL)
            {
                float t, value;
                char fn[2048] = {0};
                
                if(m_isTimeSeries)
                {
                    assert(sscanf(buffer, "%f %f", &t, &value) == 2);
    
                    m_times.push_back(t);
                    m_values.push_back(value);
                }
                else if(m_isFieldTimeSeries)
                {
                    assert(sscanf(buffer, "%f %s", &t, fn) == 2);
                    
                    m_times.push_back(t);
                    m_fieldValueFilesAtTimes.push_back(string(fn));
                }
            }
            
            fclose(f);
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  TimeSeries
     *      Method:  TimeSeries :: GetFieldValueAtTime
     * Description:  Read field values corresponding to a given time
     *--------------------------------------------------------------------------------------
     */
    void TimeSeries::GetFieldValueAtTime(int idx, vector<double> *result)
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        
        int nn = st->GetNMeshPoints();
        
        if(m_isTimeSeries)
        {
            result->resize(nn);
            
            for(int i=0; i<nn; i++)
            {
                (*result)[i++] = m_values[idx];
            }            
        }
        else if(m_isFieldTimeSeries)
        {
            ifstream ifs;
            string line;

            ifs.open(m_fieldValueFilesAtTimes[idx].c_str());
            if(!ifs.is_open())
            {
                LogError(cout << "Error: field file " << m_fieldValueFilesAtTimes[idx] << " could not be opened.." << endl);
                exit(EXIT_FAILURE);
            }
            
            /* Get number of mesh points */
            getline(ifs, line);
            int nitems = atoi(line.c_str());

            if(nitems != nn)
            {
                LogError(cout << "Incompatible input - number of nodes do not match.." << endl);
                exit(EXIT_FAILURE);
            }
            
            result->resize(nn);

            int i = 0;
            while(getline(ifs, line))
            {
                double ur = 0;

                sscanf(line.c_str(), "%lf", &ur);
                
                (*result)[i++] = ur;
            }
            ifs.close();
        }
        else
        {
            LogError(cout << "Logical error.." << endl);
            exit(EXIT_FAILURE);
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  TimeSeries
     *      Method:  TimeSeries :: GetCurrentFieldValue
     * Description:  Returns field values at node points. Note that a 2D field is returned
     *               even though the input in the parameter file may be a single scalar 
     *               value or a 1D time series - in such cases the returned 2D field is 
     *               constant-valued across the entire domain.
     *--------------------------------------------------------------------------------------
     */
    void TimeSeries::GetCurrentFieldValue(vector<double> *result)
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        float mt = m_model->GetTime();
        float dt = m_model->GetDt();
        
        int nn = st->GetNMeshPoints();
        
        /*-----------------------------------------------------------------------------
         * Single-valued 
         *-----------------------------------------------------------------------------*/
        if(m_isSingleValued)
        {
            result->resize(nn);
            
            for(int i=0; i<nn; i++)
            {
                (*result)[i] = m_value;
            }     
        }
        /*-----------------------------------------------------------------------------
         * 1D or 2D time series 
         *-----------------------------------------------------------------------------*/
        else
        {
            if(mt == dt)
            {
                GetFieldValueAtTime(m_idxLo, &m_fieldValueLo);
                GetFieldValueAtTime(m_idxLo+1, &m_fieldValueHi);
            }
            else if(mt > m_times[m_idxLo+1])
            {
                int prevIdxLo = m_idxLo;
                while( ((m_idxLo+1) < int(m_times.size()-1)) &&
                        (mt > m_times[m_idxLo+1]) )
                {
                    m_idxLo++;
                }
                
                if(prevIdxLo != m_idxLo)
                {
                    GetFieldValueAtTime(m_idxLo, &m_fieldValueLo);
                    GetFieldValueAtTime(m_idxLo+1, &m_fieldValueHi);
                }
                else
                {
                    /*-----------------------------------------------------------------------------
                     * If model-time surpasses time extent of field values/files, continue with the 
                     * last available value/file.
                     *-----------------------------------------------------------------------------*/
                }
            }

            result->resize(nn);
            
            if(mt < m_times[m_idxLo])
            {
                /* Return 0 */
                for(int i=0; i<nn; i++) (*result)[i] = 0;
            }
            else if(mt > m_times[m_idxLo+1])
            {
                /* Return last available value */
                for(int i=0; i<nn; i++) (*result)[st->O(i)] = m_fieldValueHi[i];
            }
            else
            /*-----------------------------------------------------------------------------
             * Return result after linear interpolation
             *-----------------------------------------------------------------------------*/
            {
                float f = (mt - m_times[m_idxLo]) / 
                          (m_times[m_idxLo+1] - m_times[m_idxLo]);
                
                if(f>1.) exit(EXIT_FAILURE);
                for(int i=0; i<nn; i++)
                {
                    (*result)[st->O(i)] = (f*m_fieldValueLo[i] + (1.-f)*m_fieldValueHi[i]);
                }
            }
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  TimeSeries
     *      Method:  TimeSeries :: ~TimeSeries
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    TimeSeries::~TimeSeries(){}
}}

