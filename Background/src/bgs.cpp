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

using namespace cv;
using namespace std;
using namespace bgs;


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

/*
const char* keys =
{
    "{v|video_name|input.mpg| movie file}"
    "{g|ground_truth|dir | ground truth directory}"
};
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
    "{ h   | help       | false     | Print help message }"
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
    const string bgModelName     = cmd.get<string>("model");
    const string saveName        = cmd.get<string>("save");
    const string initConfigName  = cmd.get<string>("config");
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

    //get specific point (x,y) of BG image
    int col = 253;
    int row = 308;

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
    int delay= 1000/rate;
    int cnt = 0  + shiftFrame; 
    
    
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
        
        Mat bgimg;
        bg_model.getBackgroundImage(bgimg);

        //Display sequences
        if (displayImages) { 
            imshow("image", img);
            imshow("foreground mask", fgmask);
            imshow("foreground image", fgimg);
        }

        
        /*
        //update the model
        
        char k = (char)waitKey(30);
        if( k == 27 ) break;
        if( k == ' ' )
        {
            update_bg_model = !update_bg_model;
            if(update_bg_model)
                printf("Background update is on\n");
            else
                printf("Background update is off\n");
        }
         */

        if  (cnt >= 0 ) {

        //looking for ground truth file
        if ( (compare) && (it = gt_files.find(cnt)) != gt_files.end() ) {

            gt_image = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);

            if( gt_image.data ) {

                if (displayImages)
                    imshow("GROUND TRUTH", gt_image);

                //Compare both images
                perf.pixelLevelCompare(gt_image, fgmask);

                msg.str("");
                msg     << cnt << " " << perf.asString() << " " 
                        << (int)img.at<Vec3b>(row,col)[0] << " " 
                        << (int)img.at<Vec3b>(row,col)[1] << " " 
                        << (int)img.at<Vec3b>(row,col)[2];
                outfile << msg.str() << endl;

                if (verbose) 
                    cout    << msg.str() << endl; 
            }
        }
        }

        cnt++;
        if (cv::waitKey(delay)>=0)
            break;
    }

    perf.calculateFinalPerformanceOfMetrics();

    if (!groundTruthName.empty()) {
        cout    << perf.summaryAsString() << endl;
        cout    << perf.averageSummaryAsString() << endl;
        cout    << perf.metricsStatisticsAsString() << endl;
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

