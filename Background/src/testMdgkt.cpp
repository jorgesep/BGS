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
//#include <log4cplus/logger.h>
//#include <log4cplus/loggingmacros.h>
//#include <log4cplus/configurator.h>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sstream>
#include <map>

#include "bgs.h"
#include "mdgkt_filter.h"
#include "background_subtraction.h"
#include "Performance.h"
#include "utils.h"

using namespace cv;
using namespace std;
using namespace bgs;


const char* keys =
{
    "{ i   | input      |           | Input video }"
    "{ o   | output     |           | Output video }"
    "{ g   | gtruth     |           | Input ground-truth directory }"
    "{ m   | model      |           | Load background model from  file }"
    "{ s   | save       |           | Save background model to file }"
    "{ c   | config     |           | Load init config file }"
    "{ f   | frame      | 0         | Shift ground-truth in +/- n frames, e.g -f -3 or -f 3}"
    "{ d   | display    | false     | Display video sequence }"
    "{ v   | verbose    | false     | Display output messages }"
    "{ p   | point      |           | Print out RGB values of point,  e.g -p 250,300 }"
    "{ h   | help       | false     | Print help message }"
};



int main( int argc, char** argv )
{


    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);

    if (cmd.get<bool>("help"))
    {
        cout << "Testing MDGKT Filter." << endl;
        cout << "------------------------------------------------------------------------------" << endl;
        cout << "Takes sequence of frame and applied temporal filter." << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Read input parameters
    const string inputVideoName  = cmd.get<string>("input");
    const string displayPoint   = cmd.get<string>("point");

    namedWindow("1",CV_WINDOW_NORMAL);
    namedWindow("2",CV_WINDOW_NORMAL);
    moveWindow("1",20,20);
    moveWindow("2",20,300);


    Mat image;
    Mat frame;

    
    //spatio-temporal pre-processing filter for smoothing transform
    mdgkt* preProc = mdgkt::Instance();

    string path = "/Users/jsepulve/Downloads/BGS/Background/";
    
    //frame = imread(path + "config/1.png", CV_LOAD_IMAGE_COLOR);
    frame = imread("/Users/jsepulve/Tesis/Videos/jpeg/Person1/Camera_3/00000221.jpg", CV_LOAD_IMAGE_COLOR);
    preProc->initializeFirstImage(frame);

    preProc->SpatioTemporalPreprocessing(frame, image);

    imshow("1",frame);
    imshow("2",image);

    char key = 0;
    while (key != 27)
        key = (char)waitKey(30);

    //frame = imread(path + "config/2.png", CV_LOAD_IMAGE_COLOR);
    frame = imread("/Users/jsepulve/Tesis/Videos/jpeg/Person1/Camera_3/00000222.jpg", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    
    imshow("1",frame);
    imshow("2",image);

    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
    
    
    //frame = imread(path + "config/3.png", CV_LOAD_IMAGE_COLOR);
    frame = imread("/Users/jsepulve/Tesis/Videos/jpeg/Person1/Camera_3/00000223.jpg", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    
    imshow("1",frame);
    imshow("2",image);

    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
    
    Mat roiTo1(image,Rect(40,12,6,6));
    //Mat roiTo2(image,Rect(62,12,18,6));
    //Mat roiTo3(image,Rect(76,12,18,6));
    cout << "+++++++++++++++++++++++++++++++++" << endl;
    cout << roiTo1 << endl;
    //cout << roiTo2 << endl; 
    //cout << roiTo3 << endl;
    cout << "+++++++++++++++++++++++++++++++++" << endl;

    
    //frame = imread(path + "config/4.png", CV_LOAD_IMAGE_COLOR);
    frame = imread("/Users/jsepulve/Tesis/Videos/jpeg/Person1/Camera_3/00000224.jpg", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    imshow("1",frame);
    imshow("2",image);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
    
    frame = imread(path + "config/5.png", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    imshow("1",frame);
    imshow("2",image);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
   
    frame = imread(path + "config/6.png", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    imshow("1",frame);
    imshow("2",image);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    frame = imread(path + "config/7.png", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    imshow("1",frame);
    imshow("2",image);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    frame = imread(path + "config/8.png", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    imshow("1",frame);
    imshow("2",image);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    frame = imread(path + "config/9.png", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    imshow("1",frame);
    imshow("2",image);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);

    frame = imread(path + "config/10.png", CV_LOAD_IMAGE_COLOR);
    preProc->SpatioTemporalPreprocessing(frame, image);
    imshow("1",frame);
    imshow("2",image);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    return 0;
}

