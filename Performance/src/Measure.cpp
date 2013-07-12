/*******************************************************************************
 * <Self-Adaptive Gaussian Mixture Model.>
 * Copyright (C) <2013>  <name of author>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <stdio.h>
#include <opencv2/opencv.hpp>
//#include "boost/program_options.hpp" 
#include <boost/program_options.hpp>

#include <iomanip> 

#include "Performance.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
//#include <getopt.h>

using namespace cv;
using namespace std;
using namespace bgs;
namespace po = boost::program_options;


void display_usage( void )
{
    cout
    << "------------------------------------------------------------------------------" << endl
    << "Performance Measure program."                                                   << endl
    << "------------------------------------------------------------------------------" << endl
    << endl;
    //exit( EXIT_FAILURE );
}

int main( int argc, char** argv )
{
    //declaration of local variables.
    map<unsigned int, string> gt_files;
    map<unsigned int, string> fg_files;
    map<unsigned int, string>::iterator gt_it;
    map<unsigned int, string>::iterator fg_it;
    int gt_size = -1;
    int fg_size = -1;
    bool verbose = false;
    string mask_dir;
    string ground_dir;
    string param("parameters.txt");

    /** Define and parse the program options 
     */ 
    try {
        po::options_description desc("Options");

        desc.add_options()
        ("help,h", "produce help message")
        ("verbose,v", "display messages")
        ("ground,g", po::value<string>(), "input ground-truth directory")
        ("mask,m",   po::value<string>(), "input foreground mask directory")
        ("param,p",   po::value<string>(), "file with message that contains main parameters of algorithm.")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        
        if (vm.empty()) {
            display_usage();
            cout << desc << "\n";
            return 0;
        }

        if (vm.count("help")) {
            display_usage();
            cout << desc << "\n";
            return 0;
        }
        
        if (vm.count("verbose"))
            verbose = true;
        
        if (vm.count("ground")) {
            ground_dir = vm["ground"].as<string>();
            
            // Read files from input directory
            list_files(ground_dir,gt_files);
            gt_size = gt_files.size();
            
            if (gt_size == -1 || gt_size == 0) {
                cout << "Not valid ground-truth images directory ... " << endl;
                return -1;
            }
        }
        
        if (vm.count("mask")) {
            mask_dir = vm["mask"].as<string>();
            
            param = mask_dir + "/" +  param;
            // Read files from input directory
            list_files(mask_dir,fg_files, ".jpg");
            fg_size = fg_files.size();
            
            if (fg_size == -1 || fg_size == 0) {
                cout << "Not valid foreground mask directory ... " << endl;
                return -1;
            }

        }
        
        if (vm.count("param")) {
            param = vm["param"].as<string>();
        }

    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }
    
    
    
    // Object to measure performance
    Performance *measure = new Performance();
    stringstream msg,ptmsg;
    ofstream outfile, ptfile, rocfile;

    int cnt  = 0;
    int size = 0;
    Mat gtimg;
    Mat fgmask;
    string header("# ");
    
    //Comparing size of both lists.
    //In case they are not the similar takes lower size
    size = fg_size <= gt_size ? fg_size : gt_size;
    
    // Takes ground truth as reference for counter.
    cnt = gt_files.begin()->first;
    
    //for (gt_it = gt_files.begin(); gt_it != gt_files.end(); gt_it++)
    //    cout << gt_it->first << " " << gt_it->second << endl;
    
    //Opening result file.
    ifstream infile (param.c_str());
    if (infile.is_open()) {
        stringstream lines;
        string line;
        
        while ( infile.good() ) {
            getline (infile,line);
            lines << line;
        }
        
        header = lines.str();
        
        infile.close();
    }

    outfile.open("measure.txt");
    outfile << header << endl;
    //outfile << "# " << mask_dir << " " <<  ground_dir  << endl; 

    for ( int i= cnt; i<size + cnt; i++)
    {
        if ( (gt_it = gt_files.find(i)) != gt_files.end() && (fg_it = fg_files.find(i)) != fg_files.end()) {
        
            gtimg  = Scalar::all(0);
            fgmask = Scalar::all(0);
            
            // open ground truth frame.
            gtimg = imread(gt_it->second);
            
            // open foreground mask.
            fgmask= imread(fg_it->second);
            
            if( gtimg.data && fgmask.data) {
                
                //compare both frames
                measure->pixelLevelCompare(gtimg, fgmask);
                
                //Debug messages.
                msg.str("");
                msg     << fileName(fg_it->second) << " " << measure->asString() << " " ;
                outfile << msg.str() << endl;
                
                if (verbose)
                    cout    << msg.str() << endl;

            }
        }
    }
    
    measure->calculateFinalPerformanceOfMetrics();
    

    //print out final result
    cout    << measure->metricsStatisticsAsString() << endl;
    outfile << "# TPR FPR SPE MCC  TPR TNR SPE MCC" << endl;
    outfile << "# " << measure->metricsStatisticsAsString() << endl;
    outfile.close();

    rocfile.open("measure_roc.txt", std::fstream::out | std::fstream::app);
    rocfile 
    << measure->rocAsString() << endl;
    rocfile.close();

    
    
    delete measure;

    return 0;
}

