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

const char* keys =
{
    "{v|video_name|input.mpg| movie file}"
    "{g|ground_truth|dir | ground truth directory}"
};

int main( int argc, char** argv )
{
    map<unsigned int, string> gt_files;
    map<unsigned int, string>::iterator it;
    Mat gt_image;

    stringstream msg;
    ofstream outfile;

    //get specific point (x,y) of BG image
    int col = 253;
    int row = 308;

    bool compare = false;
    Performance perf;

    CommandLineParser parser(argc, argv, keys);
    string videoName = parser.get<string>("v");
    string groundDir = parser.get<string>("g");
    parser.printParams();

    if (videoName.empty()) {
        help();
    }

    if (!groundDir.empty()) {
        compare=true;
        list_files(groundDir,gt_files);
        namedWindow("GROUND THRUTH", CV_WINDOW_NORMAL);
        outfile.open("output.txt");
    }


    mdgkt* preProc = mdgkt::Instance();
    
    //string videoName= "/Users/jsepulve/Downloads/Last_Downloads/Matlab/dvcam/testxvid.avi";
    //string videoName= "/Users/jsepulve/Documents/Universidad/usach/ProyectoTesis/Code/Matlab/dvcam/testxvid.avi"; 
    //string videoName= "/Users/jsepulve/Documents/Universidad/usach/ProyectoTesis/Code/Matlab/dvcam/WalkTurnBack-Camera_3-Person1.avi"; 
    //string videoName= "/Users/jsepulve/Documents/Universidad/usach/ProyectoTesis/Code/Matlab/dvcam/dvcam2-1.mpg"; 
    
    namedWindow("image", CV_WINDOW_NORMAL);
    namedWindow("foreground mask", CV_WINDOW_NORMAL);
    namedWindow("foreground image", CV_WINDOW_NORMAL);
    //namedWindow("mean background image", CV_WINDOW_NORMAL);
    
    
    VideoCapture video(videoName);
    
    // Check video has been opened sucessfully
    if (!video.isOpened())
        return 1;
    
    BackgroundSubtractorMOG3 bg_model;
    Mat img, fgmask, fgimg;
    bool update_bg_model = true;

    
    Mat frame;
    
    video >> frame;
    preProc->initializeFirstImage(frame);
    preProc->SpatioTemporalPreprocessing(frame, img);
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, img);
    video >> frame;
    preProc->SpatioTemporalPreprocessing(frame, img);

    
    double rate= video.get(CV_CAP_PROP_FPS);
    int delay= 1000/rate;
    
    int cnt = 0 ; 
    
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
        
        imshow("image", img);
        imshow("foreground mask", fgmask);
        imshow("foreground image", fgimg);
        //if(!bgimg.empty())
        //    imshow("mean background image", bgimg );

        
        //imshow("IMAGE", img);
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

        //look for ground thruth file
        if ( (compare) && (it = gt_files.find(cnt)) != gt_files.end() ) {

            gt_image = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);

            if( gt_image.data ) {
                imshow("GROUND THRUTH", gt_image);
                //Compare both images
                perf.pixelLevelCompare(gt_image, fgmask);

                msg.str("");
                msg << cnt << " " << perf.asString() << " " 
                    << (int)img.at<Vec3b>(row,col)[0] << " " 
                    << (int)img.at<Vec3b>(row,col)[1] << " " 
                    << (int)img.at<Vec3b>(row,col)[2];
                cout << msg.str() << endl; 
                outfile << msg.str() << endl;
            }
        }

        cnt++;
        if (cv::waitKey(delay)>=0)
            break;
    }

    if (!groundDir.empty()) {
        cout    << perf.summaryAsString() << endl;
        cout    << perf.averageSummaryAsString() << endl;
        outfile << perf.summaryAsString() << endl;
        outfile << perf.averageSummaryAsString() << endl;
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

