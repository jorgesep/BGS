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
    "{ n   | frame      | 0         | Shift ground-truth in +/- n frames, e.g -f -3 or -f 3}"
    "{ f   | filter     | true      | Apply smooth preprocessing filter, Default true.}"
    "{ d   | display    | false     | Display video sequence }"
    "{ v   | verbose    | false     | Display output messages }"
    "{ p   | point      |           | Print out RGB values of point,  e.g -p 250,300 }"
    "{ h   | help       | false     | Print help message }"
};



int main( int argc, char** argv )
{

    showMultipleImages();

    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);

    if (cmd.get<bool>("help"))
    {
        cout << "Background Subtraction Program." << endl;
        cout << "------------------------------------------------------------------------------" << endl;
        cout << "Process input video comparing with its ground truth." << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Read input parameters
    const string inputVideoName  = cmd.get<string>("input");
    const string outputVideoName = cmd.get<string>("output");
    const string groundTruthName = cmd.get<string>("gtruth");
    const string bgModelName     = cmd.get<string>("model");
    const string saveName        = cmd.get<string>("save");
    const string initConfigName  = cmd.get<string>("config");
    const string displayPoint    = cmd.get<string>("point");
    const bool displayImages     = cmd.get<bool>("display");
    const bool applyFilter       = cmd.get<bool>("filter");
    const bool verbose           = cmd.get<bool>("verbose");
    const int shiftFrame         = cmd.get<int>("frame");

    //declaration of local variables.
    map<unsigned int, string> gt_files;
    map<unsigned int, string>::iterator it;
    Mat gt_image;
    stringstream msg,ptmsg;
    ofstream outfile, ptfile;
    BackgroundSubtractorMOG3 bg_model;
    Mat img, fgmask, fgimg, ftimg,fgimg_final;
    bool update_bg_model = true;
    Mat frame;
    bool compare = false;
    bool show_point = false;
    Performance perf;
    int cntTemporalWindow = 0;
    mdgkt *filter;


    if (inputVideoName.empty()) {
        cout << "Insert video name" << endl;
        cmd.printParams();
        return 0;
    }

    if (!groundTruthName.empty()) {
        compare=true;
        list_files(groundTruthName,gt_files);
        outfile.open("output.txt");
        if (displayImages) {
            namedWindow("GROUND THRUTH", CV_WINDOW_NORMAL);
            moveWindow("GROUND THRUTH",400,300);
        }
    }

    if (!initConfigName.empty()) {
        bg_model.loadInitParametersFromFile(initConfigName);
    }

    //Print out initialization parameters.
    if (verbose)
        cout << bg_model.initParametersToString() << endl;
    
    //create video object.
    VideoCapture video(inputVideoName);
    
    // Check video has been opened sucessfully
    if (!video.isOpened())
        return 0;

    double rate= video.get(CV_CAP_PROP_FPS);
    int width  = video.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = video.get(CV_CAP_PROP_FRAME_HEIGHT);
    int delay  = 1000/rate;
 

    //Get specific point to be displayed in the image.
    //Check input point to display value
    int nl=0,nc=0;
    Point pt(0,0);
    if (!displayPoint.empty()) {
        pt = stringToPoint(displayPoint);
        nc = pt.x > width  ? width  : pt.x;
        nl = pt.y > height ? height : pt.y;
        show_point = true;

        //define outfile to save BGR pixel values
        stringstream name(""); 
        name << "pt_" << pt.x << "_" << pt.y << ".txt";
        ptfile.open(name.str().c_str());
    }

   
    // Create display windows 
    if (displayImages) { 
        namedWindow("image", CV_WINDOW_NORMAL);
        namedWindow("foreground mask", CV_WINDOW_NORMAL);
        namedWindow("foreground image", CV_WINDOW_NORMAL);
        moveWindow("image"           ,20,20);
        moveWindow("foreground image",20,300);
        moveWindow("foreground mask" ,400,20);
    } 

  
    //spatio-temporal pre-processing filter for smoothing frames.
    if (applyFilter) {
        filter = mdgkt::Instance();
        cntTemporalWindow = filter->getTemporalWindow();

        for (int i=0; i<filter->getTemporalWindow(); i++) {
            video >> frame;
            if (i==0)
                filter->initializeFirstImage(frame);
            filter->SpatioTemporalPreprocessing(frame, img);
        }
    }

    //Shift backward or forward ground thruth sequence counter.
    //for compensating pre-processed frames in the filter.
    int cnt    = 0  + shiftFrame + cntTemporalWindow; 

    // main loop 
    for(;;)
    {
        //Checks if option filter enabled.
        if (applyFilter) {
            video >> frame;
            if (frame.empty()) 
                break;

            //Applies spatial and temporal filter
            //note this filter return a Mat CV_32FC3 type.
            filter->SpatioTemporalPreprocessing(frame, img);
        }
        else
            video >> img;
       
        if( fgimg.empty() )
            fgimg.create(img.size(), img.type());
       
        //Calling background subtraction algorithm.
        bg_model(img, fgmask, update_bg_model ? -1 : 0);
        
        fgimg = Scalar::all(0);
        
        img.copyTo(fgimg, fgmask);
        
        Mat bgimg;
        bg_model.getBackgroundImage(bgimg);

        // this is just for debugging, save pixel information in a file
        if (show_point) {
            ptmsg.str("");
            ptmsg  << (int)img.at<Vec3b>(nl,nc)[0] << " " 
                   << (int)img.at<Vec3b>(nl,nc)[1] << " " 
                   << (int)img.at<Vec3b>(nl,nc)[2] ;
            ptfile << ptmsg.str() << endl;
        }

        //looking for ground truth file
        if ( cnt >=0 && compare && (it = gt_files.find(cnt)) != gt_files.end() ) {

            // open ground thruth frame.
            gt_image = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);

            if( gt_image.data ) {

                //Compare both images
                perf.pixelLevelCompare(gt_image, fgmask);

                // display ground thruth
                if (displayImages)
                    imshow("GROUND THRUTH", gt_image);

                //Print out debug messages to either file or std.
                msg.str("");
                msg     << cnt << " " << perf.asString() << " " ;
                if (show_point) 
                    msg << ptmsg.str();

                outfile << msg.str() << endl;

                if (verbose) 
                    cout    << msg.str() << endl; 
            }
        }

        //Display sequences
        if (displayImages) {
            img.convertTo(ftimg, CV_8UC3);
            if (show_point)
                circle(ftimg,pt,8,Scalar(0,0,254),-1,8);
            imshow("image", ftimg);
            imshow("foreground mask", fgmask);
            imshow("foreground image", fgimg);

            //this is just to save frame 350.
            if (cnt == 350) {
                imwrite("350_fgmask.jpg",fgmask);
                imwrite("350_fgimg.jpg",fgimg);
            }
        }

 
        cnt++;
        
        //just for debugging
        if (cnt == 200)
            continue;
        //if (cv::waitKey(delay)>=0)
        //    break;
 
        //char k = (char)waitKey(30);
        char k = (char)waitKey(delay);
        if( k == 27 ) { cout << "PRESSED BUTTON "<< endl; break;}
    }


    perf.calculateFinalPerformanceOfMetrics();

    if (!groundTruthName.empty()) {
        //cout    << perf.summaryAsString() << endl;
        //cout    << perf.averageSummaryAsString() << endl;
        cout    << perf.metricsStatisticsAsString() << endl;
        //outfile << "MeanR Sensitivity Specificity ...." << endl;
        outfile << perf.metricsStatisticsAsString() << endl;
        //outfile << perf.summaryAsString() << endl;
        //outfile << perf.averageSummaryAsString() << endl;
        outfile.close();
    }

    if (ptfile.is_open())
        ptfile.close();

    return 0;
}

