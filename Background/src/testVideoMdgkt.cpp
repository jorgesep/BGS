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
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>

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
    Mat img;

    
    //spatio-temporal pre-processing filter for smoothing transform
    mdgkt* preProc = mdgkt::Instance();

    VideoCapture video("/Users/jsepulve/Tesis/Videos/WalkTurnBack-Camera_3-Person1.avi");
    if (!video.isOpened())
        return -1;

    video >> frame;
    preProc->initializeFirstImage(frame);
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    char key = 0;
    while (key != 27)
        key = (char)waitKey(30);

    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
    
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
    
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
    
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
   
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);

    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, image);
    image.convertTo(img,CV_8UC3);
    imshow("1",frame);
    imshow("2",img);
    key = 0;
    while (key != 27)
        key = (char)waitKey(30);
 
    return 0;
}

