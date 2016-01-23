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
 *       Filename:  Log.hh
 *
 *    Description:  Log
 *
 *        Version:  1.0
 *        Created:  24/02/14 16:32:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_PARSER_LOG_HH
#define SRC_PARSER_LOG_HH

#include <sstream>
#include <vector>
#include <string>

namespace src { namespace parser {
    using namespace std;
    
    enum LogLevel { LOG_QUIET, LOG_ERROR, LOG_INFO, LOG_DEBUG };
    
    extern LogLevel logLevel;
    
    #define LogError(A) ((logLevel >= LOG_ERROR)?((A),0):(0))
    #define LogInfo(A) ((logLevel >= LOG_INFO)?((A),0):(0))
    #define LogDebug(A) ((logLevel >= LOG_DEBUG)?((A),0):(0))

    void debugBreak();
    vector<string> split(const string &s, char delim); 
}}

#endif
