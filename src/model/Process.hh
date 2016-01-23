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
 *       Filename:  Process.hh
 *
 *    Description:  Base-class
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:10:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MODEL_PROCESS_HH
#define SRC_MODEL_PROCESS_HH

#include <Config.hh>

namespace src { namespace model {
    using namespace std;
    using namespace src::parser;
    class Model;
 
    /*
     * =====================================================================================
     *        Class:  Process
     *  Description:  Base-class with a virtual 'Execute' method
     * =====================================================================================
     */
    class Process
    {
        public:
        Process(const Model *m, Config *c);
        virtual ~Process();
        virtual void Execute() = 0;

        protected:
        int   m_frequency;
        const Model *m_model;
        Config *m_config; 
    };
}}

#endif
