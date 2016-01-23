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
 *       Filename:  Config.hh
 *
 *    Description:  Config: Parses structured config files
 *                  Config files contains lines with name-value assignements in the form
 *                  "<name> = <value>". Trailing and leading whitespace is stripped. 
 *                  Parsed config entries are stored in a symbol map. Lines beginning 
 *                  with '#' are a comment and ignored.
 *
 *                  Config files may be structured (to arbitrary depth). To start a new 
 *                  config sub group (or sub section) use a line in the form of 
 *                  "<name> = [". Subsequent entries are stored in the sub group, until 
 *                  a line ending with "]" is found.
 *
 *                  Values may reuse already defined names as a variable which gets 
 *                  expanded during the parsing process. Names for expansion are 
 *                  searched from the current sub group upwards. Finally the process 
 *                  environment is searched, so also environment variables may be used as
 *                  expansion symbols in the config file.
 *
 *                  Errors and warnings are handled by emitting logging messages 
 *                  (see Log.h/Log.cpp) or by calling exit() for severe errors. 
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:58:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_PARSER_CONFIG_HH
#define SRC_PARSER_CONFIG_HH

#include <string>
#include <map>
#include <list>

namespace src { namespace parser {
    using namespace std;

    /*
     * =====================================================================================
     *        Class:  Config
     *  Description:  Configuration parser
     * =====================================================================================
     */
    class Config
    {
        public:
        /* Parse config file 'configFile'. If the process environment
        * is provided, environment variables can be used as expansion symbols.
        */
        Config(string configFile);

        ~Config();
        
        // get string config entry
        string PString(string name);

        /* get boolean config entry
        * A value of Yes/yes/YES/true/True/TRUE leads to true,
        * all other values leads to false.
        */
        bool PBool(string name);

        // get double config entry; value is parsed using atof()
        double PDouble(string name);

        // get int config entry; value is parsed using atoi()
        int PInt(string name);

        // get the symbol map (e.g. for iterating over all symbols)
        inline map<string, string>& GetSymbols()
        {
            return m_symbols;
        }

        // get config sub group
        inline Config* Group(string name)
        {
            if(m_groups.find(name) == m_groups.end()) return NULL;
            return m_groups[name];
        }

        // get config sub group map (e.g. for iterating over all groups)
        inline map<string, Config*>& GetGroups()
        {
            return m_groups;
        }
        
        string GetConfigName(){return m_debugInfo;}

        private:
        // private constructor for sub groups
        Config(string name, string parentDebugInfo);

        // helper functions for parsing
        void Add(string name, string value);
        void Split(string in, string& left, string& right, char c);
        void Trim(string& s);
        void SymbolExpand(string& s);
        void SymbolExpand(map<string, string>& symbols, string& s);
        void EnvSymbolExpand(string& s);
        string ProcessInlineComments(string line);
        
        // config group symbol map
        map<string, string> m_symbols;

        // environment symbol map
        map<string, string> m_envSymbols;

        // config sub group map
        map<string, Config*> m_groups;

        // stack of config groups for parsing (only used in top config element)
        list<Config*> m_groupStack;

        // debug info used for logging messages
        string m_debugInfo;
    };

}}
#endif

