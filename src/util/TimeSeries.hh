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
 *       Filename:  TimeSeries.hh
 *
 *    Description:  TimeSeries class
 *
 *        Version:  1.0
 *        Created:  28/12/15 15:46:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)           
 *
 * =====================================================================================
 */
#ifndef SRC_UTIL_TIMESERIES_HH
#define SRC_UTIL_TIMESERIES_HH

#include <ctime>
#include <Config.hh>

#include <vector>

namespace src { namespace model {
    class Model;
}}    

namespace src { namespace util {
    using namespace std;
    using namespace src::util;
    using namespace src::parser;
    using namespace src::model;
    
    /*
     * =====================================================================================
     *        Class:  TimeSeries
     *  Description:  Variables such as precipittion can be a: (i) single scalar value 
     *                (ii) 1D time-series specified as a file containing (time, value) pairs
     *                (iii) 2D time-series specified as a file containing (time, fieldFile) 
     *                      pairs, where fieldFile contains values at each node point in the
     *                      computational mesh.
     * =====================================================================================
     */
    class TimeSeries
    {
        public:
        TimeSeries(const Model *m, Config *c, string paramName);
        ~TimeSeries();
        void GetCurrentFieldValue(vector<double> *result);
        
        private:
        void GetFieldValueAtTime(int idx, vector<double> *result);
        void ReadFieldFile(string fn, vector<double> *result);
        void AscertainTimeSeriesType();

        const Model *m_model;
        Config *m_config;
        string m_paramName;

        double m_value;
        string m_file;
        vector<float> m_times;
        vector<float> m_values;
        vector<string> m_fieldValueFilesAtTimes;

        vector<double> m_fieldValueLo;
        vector<double> m_fieldValueHi;
        int m_idxLo;

        bool m_isSingleValued;
        bool m_isTimeSeries;
        bool m_isFieldTimeSeries;
    };
}}

#endif
