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
 *       Filename:  spgm.cc
 *
 *    Description:  Implements the main() function
 *
 *        Version:  1.0
 *        Created:  08/10/13 16:22:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <exception>
#include <stdexcept>

#include <ModelBuilder.hh>
#include <Model.hh>
#include <SurfaceTopology.hh>
#include <SurfaceTopologyOutput.hh>
#include <RegularMesh.hh>
#include <Allocator.hh>

#include <Process.hh>
#include <Config.hh>
#include <ScalarField.hh>
#include <Diffusion.hh>

using namespace std;
using src::geometry::Triangulator;
using src::mesh::SurfaceTopology;
using src::mesh::SurfaceTopologyOutput;
using src::mesh::RegularMesh;
using src::mem::PoolAllocator;
using src::model::Model;
using src::model::ModelBuilder;
using src::model::Process;
using src::parser::Config;
using namespace src::util;
using namespace src::math;

const string usage = "Usage: ./spgm <config-file>\n";

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ReadParameters
 *  Description:  Parses the configuration file and prints out all parameters
 * =====================================================================================
 */
Config *ReadParameters(string fileName)
{
    Config *c = new Config(fileName);

    /*-----------------------------------------------------------------------------
     * Create a list of mandatory basic parameters 
     *-----------------------------------------------------------------------------*/
    map<string, bool> mandatoryBasicParametersFound;
    mandatoryBasicParametersFound["dt"] = false;
    
    /*-----------------------------------------------------------------------------
     * Create a list of mandatory basic parameter-groups 
     *-----------------------------------------------------------------------------*/
    map<string, bool> mandatoryBasicParameterGroupsFound;
    mandatoryBasicParameterGroupsFound["mesh"] = false;

    /*-----------------------------------------------------------------------------
     * Check for basic and optional parameters:
     *-----------------------------------------------------------------------------*/
    try
    {
        cout << endl;
        cout << "*************************************" << endl;
        cout << "*       SPGM Model Parameters       *" << endl;
        cout << "*************************************" << endl;

        map<string, string> symbols = c->GetSymbols();
        for(map<string, bool>::iterator pit = mandatoryBasicParametersFound.begin(); 
            pit != mandatoryBasicParametersFound.end(); pit++)
        {
            for(map<string, string>::iterator sit = symbols.begin(); 
                sit != symbols.end(); sit++)
            {
                if(sit->first == pit->first) mandatoryBasicParametersFound[pit->first] = true;
            }

            if(!mandatoryBasicParametersFound[pit->first])
            {
                cerr << "Mandatory parameter '" + pit->first + "' not found.." << endl;
                throw exception();
            }
        }
        /*-----------------------------------------------------------------------------
         * Print basic parameters 
         *-----------------------------------------------------------------------------*/
        for(map<string, string>::iterator sit = symbols.begin(); 
            sit != symbols.end(); sit++)
        {
            cout << sit->first << setw(30 - sit->first.length()) 
                 << " = " << sit->second << endl << endl;
        }
        

        map<string, Config*> groups = c->GetGroups();
        for(map<string, bool>::iterator pgit = mandatoryBasicParameterGroupsFound.begin(); 
            pgit != mandatoryBasicParameterGroupsFound.end(); pgit++)
        {
            for(map<string, Config*>::iterator git = groups.begin(); 
                git != groups.end(); git++)
            {
                if(git->first == pgit->first) mandatoryBasicParameterGroupsFound[pgit->first] = true;
            }

            if(!mandatoryBasicParameterGroupsFound[pgit->first])
            {
                cerr << "Mandatory parameter-group '" + pgit->first + "' not found.." << endl;
                throw exception();
            }
        }
        /*-----------------------------------------------------------------------------
         * Print parameter groups
         *-----------------------------------------------------------------------------*/
        for(map<string, Config*>::iterator git = groups.begin(); 
            git != groups.end(); git++)
        {
            cout << git->first << " = [" << endl;
            Config *cl = git->second;
            
            map<string, string> symbolsl = cl->GetSymbols();
            for(map<string, string>::iterator sit = symbolsl.begin(); 
                sit != symbolsl.end(); sit++)
            {
                cout << "\t" << sit->first << setw(22 - sit->first.length()) 
                     << " = " << sit->second << endl;
            }
            cout << "]" << endl << endl;
        }
    }
    catch(exception e)
    {
        cerr << "Exception in basic parameter checks: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    return c;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  Main function with the simulation loop.
 * =====================================================================================
 */
int main(int argc, char **argv)
{
    /*-----------------------------------------------------------------------------
     * Parse parameter file
     *-----------------------------------------------------------------------------*/
    if(argc != 2)
    {
        cout << usage;
        exit(EXIT_FAILURE);
    }

    Config *c = ReadParameters(string(argv[1]));
    
    ModelBuilder mb(c);
    Model *m = mb.GetModel();
    
    /*-----------------------------------------------------------------------------
     * Main time-loop
     *-----------------------------------------------------------------------------*/
    SurfaceTopology *st = mb.GetSurfaceTopology();
    
    if(mb.GetSurfaceTopologyOutput()) mb.GetSurfaceTopologyOutput()->Write();
    
    while(m->NextTimeStep())
    {
        /* Iterate over processes and call execute */
        for(int pi = 0; pi<m->GetProcessCount(); pi++)
        {
            Process *process = m->GetProcess(pi);
            process->Execute();
        }

        st->UpdateNetwork();
        
        printf("Timestep: (%d), Time(%2.2f yr)\n", 
                m->GetTimeStep(), m->GetTime());
        
        if(mb.GetSurfaceTopologyOutput()) mb.GetSurfaceTopologyOutput()->Write();
    }
    
    delete c;
    return EXIT_SUCCESS;
}

