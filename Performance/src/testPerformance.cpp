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

#include <iomanip> 

#include "Performance.h"

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

using namespace cv;
using namespace std;
using namespace bgs;


void display_usage( void )
{
    cout
    << "------------------------------------------------------------------------------" << endl
    << "This test program compares two file images."                                    << endl
    << "Usage:"                                                                         << endl
    << "./testPerformance -r reference.jpg -i input.jpg" << endl
    << "--------------------------------------------------------------------------"     << endl
    << endl;
    exit( EXIT_FAILURE );
}

int main( int argc, char** argv )
{
    string reference;
    string input_image;
    static int verbose_flag;
    int option_index = 0;
    int c;
    
    static struct option long_options[] = {
        {"verbose", no_argument,       &verbose_flag, 1},
        {"brief",   no_argument,       &verbose_flag, 0},
        {"ref",  required_argument, 0, 'r'},
        {"img",  required_argument, 0, 'i'},
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long(argc, argv, "r:i:",
                            long_options, &option_index)) != -1) {
        if (c == -1)
            break;
        switch (c) {
        case 'r':
            reference = optarg;
            break;
        case 'i':
            input_image = optarg;
            break;
        case 'h':
        case '?':
            display_usage();
            break;
        default:
            abort ();
        }
        
    }
    Performance perf;

    string ref, img2, img3, img;
    if (!reference.empty() && !input_image.empty()) {
        ref = reference;
        img2= input_image;
    }
    else {
        string ref1,ref2,m_im1,m_im2,o_im1,o_im2;

        //string img  = "/Users/jsepulve/Downloads/BGS/Performance/src/reference.jpg";
        //string ref  ("/Users/jsepulve/Downloads/BGS/Performance/src/reference.jpg");
        
        ref1 = "/Users/jsepulve/Downloads/BGS/build/GT-00000216.PNG";
        ref2 = "/Users/jsepulve/Downloads/BGS/build/GT-00000217.PNG";

        m_im1 = "/Users/jsepulve/Downloads/BGS/build/matlab_fgkernel_216.png";
        m_im2 = "/Users/jsepulve/Downloads/BGS/build/matlab_fgkernel_217.png";

        o_im1 = "/Users/jsepulve/Downloads/BGS/build/216_fgmask.png";
        o_im2 = "/Users/jsepulve/Downloads/BGS/build/217_fgmask.png";

        Mat r1 = imread(ref1);
        Mat r2 = imread(ref2);

        Mat m1 = imread(m_im1);
        Mat m2 = imread(m_im2);
        
        Mat o1 = imread(o_im1);
        Mat o2 = imread(o_im2);
        
        //perf.countPixelsReferenceImage(r1);
        //cout << perf.refToString() << endl;
        
        perf.pixelLevelCompare(r1, m1);
        cout << perf.refToString() << endl;
        cout << perf.asString() << endl;
        perf.pixelLevelCompare(r2, m2);
        cout << perf.asString() << endl;

        perf.pixelLevelCompare(r1, o1);
        cout << perf.asString() << endl;
        perf.pixelLevelCompare(r2, o2);
        cout << perf.asString() << endl;
        
        //string img1 = "/Users/jsepulve/Tesis/Code/cpp/Modules/Performance/src/GT-00000363.PNG";
        //string img2 = "/Users/jsepulve/Tesis/Code/cpp/Modules/Performance/src/363.png";
    }
    
    
    Mat im;
    im = imread(img.c_str(),CV_LOAD_IMAGE_COLOR);

    /*
    if(! im.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    perf.setPixelsReference(im);
    cout << perf.refToString();
    */

    Mat im1 = imread(ref.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
    Mat im2 = imread(img2.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
    //Mat im3 = imread(img3);
    
    if (!im1.data || !im2.data )
    {
        cerr << "Could not open or find the images" << endl;
        return -1;
    }
    
    perf.countPixelsReferenceImage(im1);
    cout << perf.refToString() << endl;
    
    //Performance::ContingencyMatrix index[3];
    perf.pixelLevelCompare(im1, im2);
    cout << perf.asString() << endl;
   // index = perf.getContingencyMatrix();

    //perf.pixelLevelCompare(im1, im3);
    //cout << perf.asString() << endl;
    
    
    return 0;
}

