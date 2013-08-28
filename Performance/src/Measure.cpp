/*******************************************************************************
 * This file is part of libraries to evaluate performance of Background 
 * Subtraction algorithms.
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
#include <boost/filesystem.hpp>

#include <iomanip> 

#include "Performance.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
//#include <cstdlib>

using namespace cv;
using namespace std;
using namespace bgs;
namespace po = boost::program_options;
using namespace boost::filesystem;


static const std::string GROUNDTRUTH_ENV = "GROUNDTRUTH";
static const std::string GROUNDMAP_ENV   = "GROUNDMAP";

void display_usage( void )
{
    cout
    << "------------------------------------------------------------------------------" << endl
    << "Performance Measure program."                                                   << endl
    << "------------------------------------------------------------------------------" << endl
    << "                 --------------                                               " << endl
    << " Masks dir ---->|              |                                              " << endl
    << "                |              |                                              " << endl
    << " Truth dir ---->|  Performance |--> Measures.txt                              " << endl
    << "                |    Measure   |                                              " << endl
    << " Parameters --->|              |                                              " << endl
    << "                 --------------                                               " << endl
    << "                                                                              " << endl
    << endl;
    //exit( EXIT_FAILURE );
}

int main( int argc, char** argv )
{
    //declaration of local variables.
    map<unsigned int, string> gt_files;
    map<unsigned int, string> fg_files;
    map<unsigned int, string> map_files;
    map<unsigned int, string>::iterator gt_it;
    map<unsigned int, string>::iterator fg_it;
    map<unsigned int, string>::iterator map_it;
    vector< pair<string,string> > parameters;
    int gt_size = -1;
    int fg_size = -1;
    int map_size= -1;
    bool verbose = false;
    string mask_dir;
    string ground_truth_dir;
    string ground_map_dir;
    string parameter_file("parameters.txt");
    string value_first_parameter("");
    string output_filename("output");
    bool pixel_performance;
    bool frame_performance;

    
    
    
    /** Define and parse the program options 
     */ 
    try {
        po::options_description desc("Options");

        desc.add_options()
        ("help,h", "produce help message")
        ("verbose,v", "display messages")
        ("ground,g"            , po::value<string>(), "input ground-truth directory")
        ("input,i"              , po::value<string>(), "input foreground mask directory")
        ("map,d"              , po::value<string>(), "input ground-truth map directory to compute dscore")
        ("parameter_file,p"    , po::value<string>(), "configuration parameters file. By default looks into ground-truth dir")
        ("pixel_performance,l" , po::value<bool>(&pixel_performance)->zero_tokens()->default_value(false), "pixel performance.")
        ("frame_performace,f"  , po::value<bool>(&frame_performance)->zero_tokens()->default_value(false),"frame performance.")
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
        
        if (!pixel_performance && !frame_performance)
            pixel_performance = true;

        
        if (vm.count("verbose"))
            verbose = true;
        
        if (vm.count("ground")) {
            
            ground_truth_dir = vm["ground"].as<string>();
   
        }
        else {
            
            // Verify environment variable.
            char const* env = getenv( GROUNDTRUTH_ENV.c_str() );
            if (env != NULL) {
                ground_truth_dir = string(env);
            }
            else {
                cout << "No ground-truth directory selected." << endl;
                return -1;
            }
            
        }
            
        // Read files from input directory
        list_files(ground_truth_dir,gt_files);
        gt_size = gt_files.size();
        
        if (gt_size == -1 || gt_size == 0) {
            cout << "Not valid ground-truth images directory ... " << endl;
            return -1;
        }
        
        if (vm.count("input")) {
            
            mask_dir = vm["input"].as<string>();
            
            parameter_file = mask_dir + "/" +  parameter_file;
            if (is_regular_file(parameter_file)) {
                // Read parameters.txt and save values in vector parameters.
                parse_file(parameter_file,parameters);
                //cout << parameters[1].first << " " << parameters[1].second << endl;
            }
            
            // Read files (.png or .jpg) from input directory
            find_list_any_image_file(mask_dir,fg_files);
            fg_size = fg_files.size();
            
            if (fg_size == -1 || fg_size == 0) {
                cout << "Not valid foreground directory ... " << endl;
                return -1;
            }

        }
        
        if (vm.count("param")) {
            parameter_file = vm["param"].as<string>();
        }
        
        // Read input directory for ground truth map xml files, they are optional just for increase performance.
        if (vm.count("map")) {
            
            ground_map_dir = vm["map"].as<string>();
            
        }
        else {
            
            // Verify environment variable.
            char const* map_env = getenv( GROUNDMAP_ENV.c_str() );
            if (map_env != NULL) {
                ground_map_dir = string(map_env);
            }  
        }
        
        if (!ground_map_dir.empty()) {
            // Read files from input directory
            list_files(ground_map_dir,map_files, ".xml");
            map_size = map_files.size();

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
    if (!pixel_performance)
        // By default pixel measures are enabled.
        measure->setPixelPerformanceMeasures(false);
    if (frame_performance) {
        measure->setFramePerformanceMeasures(true);
    }
    
    
    stringstream msg,ptmsg;
    ofstream outfile, ptfile, rocfile;

    int cnt_processed = 0;
    int cnt  = 0;
    int size = 0;
    Mat gtimg;
    Mat fgmask;
    string header("# ");
    
    // Prepare first line header of output file
    if (parameters.size() > 0) {
        vector< pair<string, string> >::iterator       it  = parameters.begin();
        vector< pair<string, string> >::const_iterator end = parameters.end();
       
        value_first_parameter = it->second + " ";
        
        stringstream out;
        out << "# ";


        
        for (; it != end; ++it) {
            out << it->first << "=" << it->second << " ";
            
            // takes first character and its value as name of the output result file.
            stringstream name;
            name << "_" << it->first.at(0) << "_" << it->second;
            output_filename += name.str();

        }

        //out << "# Name TP_R TN_R    TP_M TN_M FP_M FN_M    TPR FPR FMEASURE MCC    PSNR MSSIM DSCORE ";
        header = out.str();
    }
    
    //Comparing size of both lists.
    //In case they are not the similar takes lower size
    size = fg_size <= gt_size ? fg_size : gt_size;
    
    // Takes ground-truth as reference counter.
    cnt = gt_files.begin()->first;
    
    
    //Opening result file.
    ifstream infile (parameter_file.c_str());
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


    output_filename += ".txt";
    outfile.open(output_filename.c_str());
    outfile << header << endl;

    for ( int i= cnt; i<size + cnt; i++)
    {
        if ( (gt_it = gt_files.find(i)) != gt_files.end() && (fg_it = fg_files.find(i)) != fg_files.end()) {
        
            gtimg  = Scalar::all(0);
            fgmask = Scalar::all(0);
            

            // open ground truth frame.
            try {
                gtimg = imread(gt_it->second, CV_LOAD_IMAGE_GRAYSCALE);
            }
            catch(exception& e) {
                cerr << "Exception loading ground truth mask: " << e.what() << "\n";
                continue;
            }
            catch(...) {
                cerr << "Exception of unknown type loading ground truth mask !\n";
                continue;
            }
    
             
            // open foreground mask.
            try {
                fgmask= imread(fg_it->second, CV_LOAD_IMAGE_GRAYSCALE);
            }
            catch(exception& e) {
                cerr << "Exception loading foreground mask: " << e.what() << "\n";
                continue;
            }
            catch(...) {
                cerr << "Exception of unknown type loading foreground mask !\n";
                continue;
            }
    
 
            if( gtimg.data && fgmask.data) {
                
                //compare both frames to obtain tp, tn, fn and fp values
                if (pixel_performance)
                    measure->pixelLevelCompare(gtimg, fgmask);
                
                
                // Get similarity values as PSNR, MSSIM, and DScore
                if (frame_performance) {
                    
                    Mat map;
                    //read from xml file and return mat
                    if ( (map_it = map_files.find(i)) != map_files.end()) {
                        FileStorage fs(map_it->second, FileStorage::READ);
                        fs["MAP"] >> map;
                        fs.release();
                        
                        //cout << "Reading: " << i << " file: " << map_it->first << " : " << map_it->second << endl;
                    }
                    measure->frameSimilarity(fgmask, gtimg, map);
                
                }
                
                
                //Debug messages.
                msg.str("");
                msg     << fileName(fg_it->second) << " " << measure->asString() << " " ;
                outfile << msg.str() << endl;
                
                if (verbose)
                    cout    << msg.str() << endl;

                cnt_processed++;

            }
        }
    }
    
    // Performs calculations of mean and median of accumulated metrics
    measure->calculateFinalPerformanceOfMetrics();
    

    //print out by console final result
    cout    << measure->metricsStatisticsAsString() << " " << size << "/" << cnt_processed << endl;
    
    outfile << "# " << measure->getHeaderForFileWithNameOfStatisticParameters() << endl;

    outfile << "# " << measure->metricsStatisticsAsString() << endl;
    outfile.close();

    
    rocfile.open("measure_roc.txt", std::fstream::out | std::fstream::app);
    rocfile 
    << value_first_parameter
    << measure->rocAsString() << endl;
    rocfile.close();

    
    
    delete measure;

    return 0;
}

