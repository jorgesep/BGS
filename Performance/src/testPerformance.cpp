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

    string ref, img2, img3, img;
    if (!reference.empty() && !input_image.empty()) {
        ref = reference;
        img2= input_image;
    }
    else {
        string img  = "/Users/jsepulve/Downloads/BGS/Performance/src/reference.jpg";
        string ref  ("/Users/jsepulve/Downloads/BGS/Performance/src/reference.jpg");
        string img2 ("/Users/jsepulve/Downloads/BGS/Performance/src/test1.jpg");
        string img3 = "/Users/jsepulve/Downloads/BGS/Performance/src/test2.jpg";
        //string img1 = "/Users/jsepulve/Tesis/Code/cpp/Modules/Performance/src/GT-00000363.PNG";
        //string img2 = "/Users/jsepulve/Tesis/Code/cpp/Modules/Performance/src/363.png";
    }
    Performance perf;
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

    Mat im1 = imread(ref.c_str());
    Mat im2 = imread(img2.c_str());
    //Mat im3 = imread(img3);
    
    if (!im1.data || !im2.data )
    {
        cerr << "Could not open or find the images" << endl;
        return -1;
    }
    
    //Performance::ContingencyMatrix index[3];
    perf.pixelLevelCompare(im1, im2);
    cout << perf.asString() << endl;
   // index = perf.getContingencyMatrix();

    //perf.pixelLevelCompare(im1, im3);
    //cout << perf.asString() << endl;
    
    
    return 0;
}

