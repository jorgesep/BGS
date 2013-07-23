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


#include <iostream>
#include <fstream>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

using namespace cv;
using namespace std;
//using namespace bgs;


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
    }
    Mat im1 = imread(ref.c_str());
    Mat im2 = imread(img2.c_str());
    
    if (!im1.data || !im2.data )
    {
        cerr << "Could not open or find the images" << endl;
        return -1;
    }
    
    //Mat M = (Mat_<double>(3,3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    
    Mat im1_gray;
    Mat im2_gray;
    
    //image.create(im1.size(), CV_8U);
    //brightness or luminance formula: Y=0.299R+0.587G+0.114B
    if (im1.channels() > 1)
        cvtColor( im1, im1_gray, CV_BGR2GRAY );
    if (im2.channels() > 1)
        cvtColor( im2, im2_gray, CV_BGR2GRAY );
    

    
    //remove the pixels which value = 0
    Mat ref1 = im1_gray | Mat::ones(im1_gray.size(), im1_gray.type() );
    
    //Mat dst1 = src1 >= src2;
    //Mat mask = image == 0;
    
    Mat binMat;
    // Convert image to binary.
    //cv::threshold( mask, binMat, 100, 255, THRESH_BINARY );

    
    Mat diff = im1 != im2;
   
    //----------------------------------------------
    Mat bg22   = imread("/Users/jsepulve/Downloads/BGS/Performance/config/bg22.png");
    Size size = bg22.size();
    int type  = bg22.type();
    int depth = bg22.depth();
    
    int nchannels = CV_MAT_CN(type);
    CV_Assert( nchannels == 3 );
    
    // number of lines
    int nl= bg22.rows; 
    int nc= bg22.cols*nchannels;
    int matSize = nl*nc;
    
    Mat _bgImage;
    //_bgImage.create(1, matSize, depth);
    _bgImage.create(1, matSize, CV_32F);

    
    
    if ( bg22.isContinuous() && _bgImage.isContinuous())
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }
    
    const uchar* bg22_data;
    uchar* bgImage_data;
    
    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        bg22_data    =  bg22.ptr<uchar>(j);
        bgImage_data = _bgImage.ptr<uchar>(j);
        
        for (int i=0; i<nc; i++) {
            uchar tmp       = bg22_data[i];
            tmp            += 10;
            bgImage_data[i] = tmp;
        }
    }
    
    uchar* p = bg22.data;
    cout 
    << "(0,0) : [" << (int)*(p+0) << "," << (int)*(p+1)  << ","  << (int)*(p+2) << "] "
    << "(0,1) : [" << (int)*(p+3) << "," << (int)*(p+4)  << ","  << (int)*(p+5) << "] "
    << "(1,0) : [" << (int)*(p+6) << "," << (int)*(p+7)  << ","  << (int)*(p+8) << "] "
    << "(1,1) : [" << (int)*(p+9) << "," << (int)*(p+10)  << "," << (int)*(p+11) << "] "
    << endl;

    Mat bgImg = _bgImage.reshape(nchannels,bg22.rows);
    Mat cbgImg;
    bgImg.convertTo(cbgImg, CV_8UC3);
    p = bgImg.data;
    cout 
    << "(0,0) : [" << (int)*(p+0) << "," << (int)*(p+1)  << ","  << (int)*(p+2) << "] "
    << "(0,1) : [" << (int)*(p+3) << "," << (int)*(p+4)  << ","  << (int)*(p+5) << "] "
    << "(1,0) : [" << (int)*(p+6) << "," << (int)*(p+7)  << ","  << (int)*(p+8) << "] "
    << "(1,1) : [" << (int)*(p+9) << "," << (int)*(p+10)  << "," << (int)*(p+11) << "] "
    << endl;

    
    //----------------------------------------------
    
    
    
    return 0;
}

