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
 *       Filename:  Config.cc
 *
 *    Description:  Configuration parser
 *
 *        Version:  1.0
 *        Created:  24/02/14 16:05:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <Config.hh>
#include <Log.hh>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
namespace src { namespace parser {

    using namespace src::parser;

    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: Config
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    Config::Config(string name, string parentDebugInfo)
    {
        m_debugInfo = parentDebugInfo + ", " + name;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: ProcessInlineComments
     * Description:  Parses comments
     *--------------------------------------------------------------------------------------
     */
    string Config::ProcessInlineComments(string line)
    {
        string processedLine(line);
        
        if(line[0] != '#')
        {
            size_t found = processedLine.find_first_of("#");
            
            processedLine = processedLine.substr(0, found);
        }
        
        return processedLine;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: Config
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    Config::Config(string configFile)
    {
        m_debugInfo = configFile;
        m_groupStack.push_front(this);
    
        FILE* in = fopen(configFile.c_str(), "r");
        if (!in)
        {
            cerr << "cannot open input file '" << configFile << "'" << endl;
            exit(2);
        }
    
        char buff[1024];
        while (fgets(buff, 1024, in))
        {
            string line=buff;
            line = ProcessInlineComments(line);
            if ( (line.length() > 2) && (line[0] != '#') && (line.find(')') == string::npos) )
            {
                string name;
                string value;
                Split(line, name, value, '=');
        
                if (value == "[")
                {
                    LogDebug(cout << "   config: new group '" << name << "'" << endl);
                    Config* newGroup = new Config(name, m_debugInfo);
                    m_groupStack.front()->m_groups[name] = newGroup;
                    m_groupStack.push_front(newGroup);
                } else
                {
                    for (list<Config*>::reverse_iterator i = m_groupStack.rbegin(); i != m_groupStack.rend(); ++i)
                    {
                        (*i)->SymbolExpand(value);
                    }
                    EnvSymbolExpand(value);
                    LogDebug(cout << "   config: name = '" << name << "', value = '" << value << "'" << endl);
                    m_groupStack.front()->Add(name, value);
                }
            }
        
            if ( (line.length() > 0) && (line[0] != '#') && (line.find(']') != string::npos) )
            {
                LogDebug(cout << "   end of group" << endl);
                m_groupStack.pop_front();
            }
        }
    
        fclose(in);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: ~Config
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    Config::~Config()
    {
        for (map<string, Config*>::iterator i = m_groups.begin(); i != m_groups.end(); ++i)
        {
            delete i->second;
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: Add
     * Description:  Adds name-value pairs
     *--------------------------------------------------------------------------------------
     */
    void Config::Add(string name, string value)
    {
        m_symbols[name] = value;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: Split
     * Description:  Tokenizes a string
     *--------------------------------------------------------------------------------------
     */
    void Config::Split(string in, string& left, string& right, char c)
    {
        size_t pos = in.find_first_of(c);
        if(pos == string::npos)
        {
            left = in;
            Trim(left);
            right = "";
        }
        else if (pos <= 1)
        {
            left = "";
            right = in.substr(pos+1, string::npos);
            Trim(right);
        }
        else
        {
            left = in.substr(0, pos-1);
            Trim(left);
            right = in.substr(pos+1, string::npos);
            Trim(right);
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: Trim
     * Description:  Trims a string
     *--------------------------------------------------------------------------------------
     */
    void Config::Trim(string& s)
    {
        while ( (s.length() > 1) && ( (s[0] == ' ') || (s[0] =='\t') ) )
        {
            s = s.substr(1, string::npos);
        }
        while ( (s.length() > 1) &&
            ( (s[s.length()-1] == ' ') ||
            (s[s.length()-1] == '\t') ||
            (s[s.length()-1] == '\n') ||
            (s[s.length()-1] == '\r') ) )
        {
            s = s.substr(0, s.length()-1);
        }
        if ( (s.length() > 1) && (s[0] == '"') )
        {
            s = s.substr(1, string::npos);
        }
        if ( (s.length() > 1) && (s[s.length()-1] == '"') )
        {
            s = s.substr(0, s.length()-1);
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: SymbolExpand
     * Description:  Expands a symbol
     *--------------------------------------------------------------------------------------
     */
    void Config::SymbolExpand(string& s)
    {
        SymbolExpand(m_symbols, s);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: EnvSymbolExpand
     * Description:  Expand root-level parameters
     *--------------------------------------------------------------------------------------
     */
    void Config::EnvSymbolExpand(string& s)
    {
        SymbolExpand(m_envSymbols, s);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: SymbolExpand
     * Description:  Expand symbols
     *--------------------------------------------------------------------------------------
     */
    void Config::SymbolExpand(map<string, string>& symbols, string& s)
    {
        bool expanded;
        do
        {
            expanded = false;
            for (map<string, string>::iterator i = symbols.begin(); i != symbols.end(); ++i)
            {
                string search = "%" + i->first + "%";
                string replace = i->second;
                size_t pos = s.find(search);
                if (pos != string::npos)
                {
                    expanded = true;
                    s.replace(pos, search.length(), replace);
                }
            }
        } while (expanded);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: PString
     * Description:  Returns the string value of a symbol
     *--------------------------------------------------------------------------------------
     */
    string Config::PString(string name)
    {
        map<string, string>::iterator i = m_symbols.find(name);
        if (i == m_symbols.end())
        {
            LogError(cout << "access of missing property '" << name << "' (" << m_debugInfo << ")" << endl);
            exit(4);
        }
        return i->second;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: PBool
     * Description:  Returns boolean value of a symbol
     *--------------------------------------------------------------------------------------
     */
    bool Config::PBool(string name)
    {
        string val = PString(name);
    
        if ( (val == "1")   ||
             (val == "yes") ||
             (val == "Yes") ||
             (val == "YES") ||
             (val == "true") ||
             (val == "True") ||
             (val == "TRUE") )
        {
            return true;
        }
    
        return false;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: PDouble
     * Description:  Returns double value of a symbol
     *--------------------------------------------------------------------------------------
     */
    double Config::PDouble(string name)
    {
        string val = PString(name);
    
        return atof(val.c_str());
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Config
     *      Method:  Config :: PInt
     * Description:  Returns integer value of a symbol
     *--------------------------------------------------------------------------------------
     */
    int Config::PInt(string name)
    {
        string val = PString(name);
    
        return atoi(val.c_str());
    }
}}

