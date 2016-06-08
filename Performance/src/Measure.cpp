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
#include "BGSTimer.h"

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

void mergeTwoImages(InputArray _im1, InputArray _im2, OutputArray img)
{

    vector<Mat> Imgs;
    Imgs.push_back(_im1.getMat());
    Imgs.push_back(_im2.getMat());
    //int numberImages = 2;

    if ( Imgs[0].empty() || Imgs[1].empty() ) return;


    // w - Maximum number of images in a row 
    // h - Maximum number of images in a column 
    int w = 2;
    int h = 1;
    int size = 300;
    
    // Create a new 1 channel image
    //img.create(Size(100 + size*w, 60 + size*h),CV_8U);
    img.create(Size(60 + size*w, 5 + size*h),CV_8U);
    Mat mask = img.getMat();
    mask     = Scalar::all(255);

    int m = 20;
    int n = 20; 
    for (vector<Mat>::iterator it = Imgs.begin() ; it != Imgs.end(); ++it) {


        // Find the width and height of the image
        int x = it->cols;
        int y = it->rows;
        
        // Find whether height or width is greater in order to resize the image
        int max = (x > y)? x: y;
        
        // Find the scaling factor to resize the image
        float scale = (float) ( (float) max / size );
        
        // Used to Align the images
        //if( i % w == 0 && m!= 20) {
        //    m = 20;
        //    n+= 20 + size;
        //}


        //Make a rectangle
        Rect roi( m, n, (int)( x/scale ), (int)( y/scale ) );

        // Set the image ROI to display the current image
        //Point a cv::Mat header at it (no allocation is done)
        Mat ImageROI = mask(roi);

        // Resize the input image and copy the it to the Single Big Image
        resize(*it, ImageROI, ImageROI.size(), 0, 0);   

        m += (20 + size);

    }
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
    int offset  =  0; 
    bool verbose = false;
    bool show_masks = false;
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
        ("show,s",    "display images")
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
        if (vm.count("show"))
            show_masks = true;
        
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

            /// Compute offset between ground-truth and fg masks
            /// If first frame of gt greater than first fg mask, then
            /// negative offset otherwise positive.
            offset = fg_files.begin()->first - gt_files.begin()->first ;

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
    std::ofstream outfile, rocfile;

    int cnt_processed = 0;
    int cnt  = 0;
    int size = 0;
    Mat gtimg;
    Mat fgmask;
    string header("#");
    
    /// Prepare name of output file composed by name and number algorithm parameters.
    if (parameters.size() > 0) {
        vector< pair<string, string> >::iterator       it  = parameters.begin();
        vector< pair<string, string> >::const_iterator end = parameters.end();
       
        value_first_parameter = it->second + " ";
        
        stringstream out;
        out << "#";


        
        for (; it != end; ++it) {
            out << it->first << "=" << it->second << " ";
            
            // takes first character and its value as name of the output result file.
            stringstream parameter_value_pair;
            parameter_value_pair << "_" << it->first.at(0) << "_" << it->second;
            output_filename += parameter_value_pair.str();

        }

        //out << "#Frame TP TN    TP TN FP FN    TPR FPR FMEASURE MCC    PSNR MSSIM DSCORE";
        header = out.str();
    }
    
    //Comparing size of both lists.
    //In case they are not the similar takes lower size
    size = fg_size <= gt_size ? fg_size : gt_size;
    
    // Takes ground-truth as reference counter.
    cnt = gt_files.begin()->first;
    
    
    /// Opening file which contains parameters of algorithm
    /// Appendt it to first line of output file as a header.
    std::ifstream infile (parameter_file.c_str());
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
    outfile << "#GT_Frame Frame TP TN    TP TN FP FN    TPR FPR FMEASURE MCC    PSNR MSSIM DSCORE" << endl;


    // Prepare windows to display foreground and ground masks.
    if (show_masks) {
        /// Create Windows
        namedWindow("PMBGS", CV_WINDOW_NORMAL);
        moveWindow("PMBGS",50,50);
    }

    // Register time duration
    // Create filename to register elapsed time.
    stringstream duration_file_stream;
    duration_file_stream << "Performance_" << 
                            path(bgs::chomp(ground_truth_dir)).filename().string() << ".txt" ;
    // Start timer
    BGSTimer::Instance()->setSequenceName(duration_file_stream.str(), header) ;
    BGSTimer::Instance()->registerStartTime();


    for ( int i= cnt; i<size + cnt; i++)
    {
        if ( (gt_it = gt_files.find(i)) != gt_files.end() && (fg_it = fg_files.find(i+offset)) != fg_files.end()) {
        
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
                    //read from xml directory and return mat
                    if ( (map_it = map_files.find(i)) != map_files.end()) {

                        measure->readDScoreMapFromFile(map_it->second, map);
                        
                        //cout << "Reading: " << i << " file: " << map_it->first << " : " << map_it->second << endl;
                    }
                    measure->frameSimilarity(fgmask, gtimg, map);
                
                }
                
                
                //Debug messages.
                msg.str("");
                msg     << fileName(gt_it->second) << " "  << fileName(fg_it->second) << " " << measure->asString() << " " ;
                outfile << msg.str() << endl;
                
                if (verbose)
                    cout    << msg.str() << endl;

                // Display ground-truth and foreground mask.
                if (show_masks) {
                    Mat img ;
                    mergeTwoImages(gtimg, fgmask, img);
                    if ( i == (cnt + (int)(size/2)) )
                        imwrite("two_masks.png",img);

                    imshow("PMBGS", img);

                    char key  = 0;
                    int delay = 25;
                    key = (char)waitKey(delay);
                    // Exit program
                    if( key == 27 ) break;
                }

                cnt_processed++;

            }
        }
    }


    // Return elapsed time and save it in a file.
    BGSTimer::Instance()->registerStopTime();
    cout << BGSTimer::Instance()->getSequenceElapsedTime() << endl;
    BGSTimer::deleteInstance();
    
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

