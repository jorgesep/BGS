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

#include "icdm_model.h"

using namespace cv;
using namespace std;
using namespace bgs;


const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ o | output  |       | Output video }"
    "{ g | gtruth  |       | Input ground-truth directory }"
    "{ r | read    |       | Read background model from  file }"
    "{ w | write   |       | Write background model in a file }"
    "{ c | config  |       | Load init config file }"
    "{ n | frame   | 0     | Shift ground-truth in +/- n frames, e.g -n -3 or -n 3}"
    "{ f | filter  | true  | Apply smooth preprocessing filter, Default true.}"
    "{ p | point   |       | Print out RGB values of point,  e.g -p 250,300 }"
    "{ v | verbose | false | Print out output messages by console}"
    "{ d | debug   | false | Debug mode, print by console internal gaussian parameters }"
    "{ s | show    | false | Show images of video sequence }"
    "{ h | help    | false | Print help message }"
};



int main( int argc, char** argv )
{


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
    const string bgModelName     = cmd.get<string>("read");
    const string saveName        = cmd.get<string>("write");
    const string initConfigName  = cmd.get<string>("config");
    const string displayPoint    = cmd.get<string>("point");
    const int shiftFrame         = cmd.get<int>("frame");
    const bool displayImages     = cmd.get<bool>("show");
    const bool applyFilter       = cmd.get<bool>("filter");
    const bool debugPoint        = cmd.get<bool>("debug");
    bool verbose                 = cmd.get<bool>("verbose");

    //declaration of local variables.
    map<unsigned int, string> gt_files;
    map<unsigned int, string> im_files;
    map<unsigned int, string>::iterator it;
    map<unsigned int, string>::iterator it_im;
    Mat gt_image;
    stringstream msg,ptmsg;
    ofstream outfile, ptfile, rocfile;
    BackgroundSubtractorMOG3 bg_model;
    Mat img, fgmask, fgimg, ftimg,fgimg_final;
    bool update_bg_model = true;
    Mat frame;
    bool compare = false;
    bool show_point = false;
    Performance perf;
    int cntTemporalWindow = 0;
    int gt_cnt = 0;
    int gt_size = -1;
    int im_size = -1;
    mdgkt *filter;
    bool processing_video = false;

    
    if (inputVideoName.empty()) {
        cout << "Insert video name" << endl;
        cmd.printParams();
        return -1;
    }
    
    //Check if input name if either a video or directory of jpeg files
    if (FileExists(inputVideoName.c_str())) {
        processing_video = true;
    }
    else {
        // Read files from input deirectory
        list_files(inputVideoName,im_files, ".jpg");
        im_size = im_files.size();
            
        if (im_size == -1 || im_size == 0) {
            cout << "Not valid ground images directory ... " << endl;
            return -1;
        }
    }    

    //Load initialization parameters
    if (!initConfigName.empty()) {
        bg_model.loadInitParametersFromFile(initConfigName);
    }

    //Print out initialization parameters.
    if (verbose)
        cout << bg_model.initParametersToString() << endl;

    //Check ground-truth
    if (!groundTruthName.empty()) {
        compare=true;
        list_files(groundTruthName,gt_files);
        outfile.open("output.txt");
        outfile << bg_model.initParametersAsOneLineString() << endl;

        if (displayImages) {
            namedWindow("GROUND TRUTH", CV_WINDOW_NORMAL);
            moveWindow("GROUND TRUTH",400,300);
        }
        gt_size = gt_files.size();
        
        if (gt_size == 0) {
            cout << "Not valid ground truth directory ... " << endl;
            return -1;
        }
    }

    int width;
    int height;
    int delay;
    VideoCapture video;
    
    if (processing_video) {
        //create video object.
        //VideoCapture video(inputVideoName);
        video.open(inputVideoName);
        
        // Check video has been opened sucessfully
        if (!video.isOpened())
            return 0;
        
        double rate   = video.get(CV_CAP_PROP_FPS);
        width         = video.get(CV_CAP_PROP_FRAME_WIDTH);
        height        = video.get(CV_CAP_PROP_FRAME_HEIGHT);
        delay         = 1000/rate;
    }
    else {
        //create video object.
        Mat imfile = imread(im_files[1]);
        
        // Check file has been opened sucessfully
        if (imfile.data == NULL )
            return 0;
        
        width  = imfile.cols;
        height = imfile.rows;
        delay  = 25;
    }

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
    
    //check if debug option is enabled
    //this will disable verbose mode
    if (debugPoint) {
        cout << "Debug" << endl; 
        if ( pt.x == 0 && pt.y == 0 )
            pt = Point(width/2,height/2);
        
        //stop printing out messages
        verbose = false;
            
        bg_model.setPointToDebug(pt);
        
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

            frame = Scalar::all(0);
            if (processing_video)
                video >> frame;
            else
                frame = imread(im_files[i+1]);
            
            // Initialize in zero three channels of img kernel.
            if (i==0)
                filter->initializeFirstImage(frame);
            
            //Apply filter and puts result in img.
            //Note this filter also keeps internal copy of filter result.
            filter->SpatioTemporalPreprocessing(frame, img);
            

        }
    }
    
    bg_model.initializeModel(img);
    //bg_model.loadModel();

    
    //Shift backward or forward ground truth sequence counter.
    //for compensating pre-processed frames in the filter.
    int cnt    = 0  + shiftFrame + cntTemporalWindow; 


    // main loop 
    for(;;)
    {
        //clean up all Mat structures.
        //This is done because, performance was improved.
        img    = Scalar::all(0);
        frame  = Scalar::all(0);
        fgmask = Scalar::all(0);
        
        //Checks if filter option was enabled.
        if (applyFilter) {

            if (processing_video)
                video >> frame;
            else
                frame = imread(im_files[cnt+1]);

            if (frame.empty()) 
                break;

            //Applies spatial and temporal filter
            //note this filter return a Mat CV_32FC3 type.
            filter->SpatioTemporalPreprocessing(frame, img);
        }
        else {
            if (processing_video)
                video >> img;
            else
                img = imread(im_files[cnt+1]);
        } 
            
        if( fgimg.empty() )
            fgimg.create(img.size(), img.type());
       
        //Calling background subtraction algorithm.
        bg_model(img, fgmask, update_bg_model ? -1 : 0);
        
        fgimg = Scalar::all(0);
        
        img.copyTo(fgimg, fgmask);
       
        //Get background image
        //Mat bgimg;
        //bg_model.getBackground(bgimg);
        //bg_model.getBackgroundImage(bgimg);


        //looking for ground truth file
        if ( cnt >=0 && compare && (it = gt_files.find(cnt)) != gt_files.end() ) {

            // open ground truth frame.
            gt_image = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);

            if( gt_image.data ) {

                //Compare both images
                perf.pixelLevelCompare(gt_image, fgmask);

                // display ground truth
                if (displayImages)
                    imshow("GROUND TRUTH", gt_image);

                //Print out debug messages to either file or std.
                msg.str("");
                msg     << cnt << " " << perf.asString() << " " ;
                if (show_point) 
                    msg << ptmsg.str();

                outfile << msg.str() << endl;

                if (verbose) 
                    cout    << msg.str() << endl; 
            }

            // counter of number of frame processed
            gt_cnt++;
        }
        
        // Save pixel information in a local file
        if (show_point) {
            ptmsg.str("");
            ptmsg  << (int)img.at<Vec3f>(nl,nc)[0] << " " 
                   << (int)img.at<Vec3f>(nl,nc)[1] << " " 
                   << (int)img.at<Vec3f>(nl,nc)[2] ;
            ptfile << ptmsg.str() << endl;
        }


        //Display sequences
        if (displayImages) {
            img.convertTo(ftimg, CV_8UC3);
            if (show_point){
                circle(ftimg,pt,8,Scalar(0,0,254),-1,8);
                circle(fgmask,pt,8,Scalar(155,155,155),-1,8);
            }
            
            imshow("image", ftimg);
            imshow("foreground mask", fgmask);
            imshow("foreground image", fgimg);
            //imshow("background image", bgimg);

            //this is just to save frame 350.
            if (cnt == 350) {
                imwrite("350_fgmask.jpg",fgmask);
                imwrite("350_fgimg.jpg",fgimg);
            }
        }

 
        cnt++;
        
        char key=0;
        if (displayImages)
            key = (char)waitKey(delay);
        if( key == 27 ) { cout << "PRESSED BUTTON "<< endl; break;}

        //in case of not display option enabled stop execution 
        //after last ground-truth file was processed
        if (!displayImages && compare && gt_cnt >= gt_size)
            break;

        //save model
        if (cnt == 500)
            bg_model.saveModel();
        
    }


    perf.calculateFinalPerformanceOfMetrics();

    if (!groundTruthName.empty()) {
        cout    << perf.metricsStatisticsAsString() << endl;
        outfile << "#TPR TNR SPE MCC  TPR TNR SPE MCC" << endl;
        outfile << "#" << perf.metricsStatisticsAsString() << endl;
        outfile.close();
        rocfile.open("roc.txt", std::fstream::out | std::fstream::app);
        rocfile << bg_model.getRange() << " " 
                << bg_model.getAlpha() << " "  
                << perf.rocAsString() << endl;
        rocfile.close();
    }

    if (ptfile.is_open())
        ptfile.close();

    return 0;
}

