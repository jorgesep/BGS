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

#include "opencv2/video/background_segm.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/opencv.hpp>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iomanip> 

#include <boost/filesystem.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "Performance.h"
//#include "FrameReaderFactory.h"
#include "utils.h"


using namespace cv;
using namespace std;
using namespace bgs;
using namespace boost::filesystem;

void display_usage( void )
{
    cout
    << "------------------------------------------------------------------------------" << endl
    << "Create special maps to measure DScore."                                    << endl
    << "Usage:"                                                                         << endl
    << "./dscoreMap -i reference.jpg " << endl
    << "--------------------------------------------------------------------------"     << endl
    << endl;
    exit( EXIT_FAILURE );
}

int main( int argc, char** argv )
{
    string reference;
    string input_dir;
    string ground_truth_dir;
    static int verbose_flag;
    int option_index = 0;
    int c;
   
    
    static struct option long_options[] = {
        {"verbose", no_argument,       &verbose_flag, 1},
        {"brief",   no_argument,       &verbose_flag, 0},
        {"img",  required_argument, 0, 'i'},
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long(argc, argv, "r:i:",
                            long_options, &option_index)) != -1) {
        if (c == -1)
            break;
        switch (c) {
        case 'i':
            ground_truth_dir = optarg;
            break;
        case 'h':
        case '?':
            display_usage();
            break;
        default:
            abort ();
        }
        
    }


    // Read files from input directory
    string output_dir ("XmlMap");
    
    map<unsigned int, string> gt_files;
    int gt_size = -1;

    // Verify input name is a video file or sequences of jpg files
    path path_to_ground_truth (ground_truth_dir.c_str());

    if (is_directory(path_to_ground_truth)) {

        list_files(ground_truth_dir,gt_files);
        gt_size = gt_files.size();
        
        path::iterator it(path_to_ground_truth.end());
        it--;
        output_dir += it->c_str();
        

    } 
    else {
        cout << "Invalid ground-truth directory ... "<< endl;
        return 0;
    }


    // Create local directory to save xml maps.
    path p (output_dir);
    
    if ( !exists(p) )
        create_directory(p);

    

    map<unsigned int, string>::iterator gt_it;

    Performance *measure = new Performance();

    Mat Image;
    Mat Map;
    
    
    for (gt_it = gt_files.begin(); gt_it != gt_files.end(); ++gt_it) {
        
        Image  = Scalar::all(0);
        Map    = Scalar::all(0);
        
        // open ground truth frame.
        Image = imread(gt_it->second, CV_LOAD_IMAGE_GRAYSCALE);
        
        // get general Map
        measure->computeGeneralDSCoreMap(Image, Map);
        
        string dirname = fileName(gt_it->second);
        cout << dirname << endl;

        
        stringstream mapfile;
        mapfile << output_dir << "/" << gt_it->first << ".xml";
        FileStorage fs(mapfile.str(), FileStorage::WRITE);
        stringstream tagname;
        tagname << gt_it->first;
        fs << "MAP" << Map;
        fs.release();
        
        // just for testing
        if (gt_it->first == 300) {
            
            Mat mask;
            FileStorage fsread(mapfile.str(), FileStorage::READ);
            fsread["MAP"] >> mask;
            fsread.release();
            
            normalize(mask, mask, 0, 255, cv::NORM_MINMAX);
            imwrite("300.png", mask);
            
        }
        
    }
    
    delete measure;

    

    return 0;
}

