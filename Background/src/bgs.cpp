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

/*
string chomp (string dir)
{
    unsigned found = dir.find_last_of("/");
    if (found == dir.size()-1)
        return dir.substr(0,found);
    return dir;
}

unsigned int name_to_number(string file)
{
    unsigned int sn;
    string gt ("-");
    string str(".");
    //remove .PNG
    size_t pos  = file.find(gt); 
    size_t size = file.substr(pos+1).find_last_of(str);
    string number = file.substr(pos+1,size);
    stringstream tmpstr(number);
    tmpstr >> sn;
    //cout << number << ": " << sn << endl;
    return sn;

}

void list_files(string directory, map<unsigned int,string>& list)
{
    DIR *dir;
    struct dirent *entity;

    dir = opendir(directory.c_str());
    if (dir != NULL){
        while ( (entity = readdir(dir)) ){
            const string file_name = entity->d_name;
            const string full_file_name = chomp(directory) + "/" + file_name;

            if(entity->d_type == DT_DIR)
                continue;
            if(entity->d_type == DT_REG) {
                size_t found = file_name.find(".PNG");
                if (found!=std::string::npos)
                    list[name_to_number(entity->d_name)]=full_file_name; 
           }
        }
    }
}





static void help()
{
    cout
    << "------------------------------------------------------------------------------" << endl
    << "This test program compares two file images."                                    << endl
    << "Usage:"                                                                         << endl
    << "./main -v input_video -g ground truth images" << endl
    << "--------------------------------------------------------------------------"     << endl
    << endl;
    exit( EXIT_FAILURE );
}
*/


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
    const string pointPosition   = cmd.get<string>("point");
    const bool displayImages     = cmd.get<bool>("display");
    const bool verbose           = cmd.get<bool>("verbose");
    const int shiftFrame         = cmd.get<int>("frame");

    //declaration of local variables.
    map<unsigned int, string> gt_files;
    map<unsigned int, string>::iterator it;
    Mat gt_image;
    stringstream msg;
    ofstream outfile;
    BackgroundSubtractorMOG3 bg_model;
    Mat img, fgmask, fgimg;
    bool update_bg_model = true;
    Mat frame;
    bool compare = false;
    Performance perf;


    if (inputVideoName.empty()) {
        cout << "Insert video name" << endl;
        cmd.printParams();
        return 0;
    }

    if (!groundTruthName.empty()) {
        compare=true;
        list_files(groundTruthName,gt_files);
        namedWindow("GROUND TRUTH", CV_WINDOW_NORMAL);
        outfile.open("output.txt");
    }

    if (!initConfigName.empty()) {
        bg_model.loadInitParametersFromFile(initConfigName);
    }

    //Print out initialization parameters.
    cout << bg_model.initParametersToString() << endl;
    
    // Create display windows 
    if (displayImages) { 
        namedWindow("image", CV_WINDOW_NORMAL);
        namedWindow("foreground mask", CV_WINDOW_NORMAL);
        namedWindow("foreground image", CV_WINDOW_NORMAL);
    } 
   
    //create video object.
    VideoCapture video(inputVideoName);
    
    // Check video has been opened sucessfully
    if (!video.isOpened())
        return 0;

    double rate= video.get(CV_CAP_PROP_FPS);
    int width  = video.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = video.get(CV_CAP_PROP_FRAME_HEIGHT);
    int delay  = 1000/rate;
    //The frame counter will shifted 'n' backward/forward positions 
    //of ground thruth sequence.
    int cnt    = 0  + shiftFrame; 
 

    //Get specific to be displayed in the image.
    //Check input point to display value
    int nl=0,nc=0;
    Point pt(0,0);
    if (!pointPosition.empty()) {
        pt = stringToPoint(pointPosition);
        nc = pt.x > width  ? width  : pt.x;
        nl = pt.y > height ? height : pt.y;
    }

   
    
    //spatio-temporal pre-processing filter for smoothing transform
    mdgkt* preProc = mdgkt::Instance();

    video >> frame;
    preProc->initializeFirstImage(frame);
    preProc->SpatioTemporalPreprocessing(frame, img);
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, img);
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, img);
    
    // main loop 
    for(;;)
    {
        video >> img;
        //video >> frame;
        
        //preProc->SpatioTemporalPreprocessing(frame, img);
        
        if( img.empty() )
            break;
        
        if( fgimg.empty() )
            fgimg.create(img.size(), img.type());
       
        //Calling background subtraction algorithm.
        bg_model(img, fgmask, update_bg_model ? -1 : 0);
        
        fgimg = Scalar::all(0);
        
        img.copyTo(fgimg, fgmask);
        
        //Mat bgimg;
        //bg_model.getBackgroundImage(bgimg);


        //looking for ground truth file
        if ( cnt >=0 && compare && (it = gt_files.find(cnt)) != gt_files.end() ) {

            gt_image = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);

            if( gt_image.data ) {

                if (displayImages)
                    imshow("GROUND TRUTH", gt_image);

                //Compare both images
                perf.pixelLevelCompare(gt_image, fgmask);

                //uchar* dsp_point = img.ptr<uchar>(nl);

                //msg.str("");
                //msg     << cnt << " " << perf.asString() << " " 
                //        << (int)dsp_point[nc] << " " 
                //        << (int)dsp_point[nc+1] << " " 
                //        << (int)dsp_point[nc+2];
 

                //msg.str("");
                msg.str("");
                //msg.str(   cnt + " " + perf.asString() + " " );
                msg     << cnt << " " << perf.asString() << " " 
                        << (int)fgmask.at<Vec3b>(nl,nc)[0] << " " 
                        << (int)fgmask.at<Vec3b>(nl,nc)[1] << " " 
                        << (int)fgmask.at<Vec3b>(nl,nc)[2];

                outfile << msg.str() << endl;

                if (verbose) 
                    cout    << msg.str() << endl; 
            }
        }

        //Display sequences
        if (displayImages) {
            circle(fgmask,pt,8,Scalar(255,255,254),-1,8);
            //circle(fgmask,Point(col,row),4,Scalar(0,0,254),-1,4);
            imshow("image", img);
            imshow("foreground mask", fgmask);
            imshow("foreground image", fgimg);
        }

 
        cnt++;
        if (cv::waitKey(delay)>=0)
            break;
 
        //char k = (char)waitKey(30);
        //if( k == 27 ) break;
        //if( k == ' ' )
        //{
        //    update_bg_model = !update_bg_model;
        //    if(update_bg_model)
        //        printf("Background update is on\n");
        //    else
        //        printf("Background update is off\n");
        //}
    }

    perf.calculateFinalPerformanceOfMetrics();

    if (!groundTruthName.empty()) {
        cout    << perf.summaryAsString() << endl;
        cout    << perf.averageSummaryAsString() << endl;
        cout    << perf.metricsStatisticsAsString() << endl;
        outfile << "# MeanR Sensitivity Specificity ...." << endl;
        outfile << "#" << perf.metricsStatisticsAsString() << endl;
        //outfile << perf.summaryAsString() << endl;
        //outfile << perf.averageSummaryAsString() << endl;
        outfile.close();
    }


    /*
    Mat frame;
    Mat img;
    
    video >> frame;
    preProc->initializeFirstImage(frame);
    preProc->SpatioTemporalPreprocessing(frame, img);
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, img);
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, img);
    
    
    ofstream myfile;
    myfile.open ("example.txt");
    
    vector<Mat> channels;
    split(img,channels);
    myfile << "KERNEL WEIGHT(0): " << endl << " " << channels.at(0) << endl << endl;
    myfile << "KERNEL WEIGHT(1): " << endl << " " << channels.at(1) << endl << endl;
    myfile << "KERNEL WEIGHT(2): " << endl << " " << channels.at(2) << endl << endl;
    myfile.close();
    */
    return 0;
}

