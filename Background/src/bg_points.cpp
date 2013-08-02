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
//#include <log4cplus/logger.h>
//#include <log4cplus/loggingmacros.h>
//#include <log4cplus/configurator.h>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sstream>
#include <map>

/*
#include "bgs.h"
#include "mdgkt_filter.h"
#include "background_subtraction.h"
#include "Performance.h"
*/

using namespace cv;
using namespace std;
//using namespace bgs;


string chomp (string dir)
{
    unsigned found = dir.find_last_of("/");
    if (found == dir.size()-1)
        return dir.substr(0,found);
    return dir;
}




const char* keys =
{
    "{ v   | video      |           | Input video }"
    "{ o   | output     |           | Output video }"
    "{ p1  | point1     |           | Point 1 position, e.g -p1 250,300 }"
    "{ p2  | point2     |           | Point 2 position, e.g -p1 250,300 }"
    "{ p3  | point3     |           | Point 3 position, e.g -p1 250,300 }"
    "{ p4  | point4     |           | Point 4 position, e.g -p1 250,300 }"
    "{ h   | help       | false     | Print help message }"
};



int main( int argc, char** argv )
{
    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);

    if (cmd.get<bool>("help"))
    {
        cout << "Background Program." << endl;
        cout << "------------------------------------------------------------------------------" << endl;
        cout << "Get four point data of background sequence." << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Read input parameters
    const string inputVideoName  = cmd.get<string>("video");
    const string outputVideoName = cmd.get<string>("output");
    const string point1          = cmd.get<string>("point1");
    const string point2          = cmd.get<string>("point2");
    const string point3          = cmd.get<string>("point3");
    const string point4          = cmd.get<string>("point4");

    if (inputVideoName.empty()) {
        cout << "Insert video name" << endl;
        cmd.printParams();
        return 0;
    }

    //create video object.
    VideoCapture video(inputVideoName);
    
    // Check video has been opened sucessfully
    if (!video.isOpened())
        return 0;


    double rate= video.get(CV_CAP_PROP_FPS);
    int delay= 1000/rate;

    //Point pt1 =  Point(250, 300);
    Point pt1 =  Point(244,680);
    //Point pt2 =  Point(680,244);
    //Point pt3 =  Point(10, 8);
    //Point pt4 =  Point(10, 8);
    //image.at<cv::Vec3b>(j,i)[0]= 255;
    //image.at<cv::Vec3b>(j,i)[0]= 255;

    if (!point1.empty()) {
        size_t pos = point1.find(',');
        size_t end = point1.size() - pos;
        stringstream X1( point1.substr(0,pos) );
        stringstream Y1( point1.substr(pos+1,end) );
        int x;
        int y;
        X1 >> x;
        Y1 >> y;
        pt1.x = x;
        pt1.y = y;
    }


    stringstream ss;
    ss << "point_" << pt1.x << "_" << pt1.y;
    string title(ss.str());
    namedWindow(title, CV_WINDOW_NORMAL);
    
    string fileName(ss.str() + ".txt");
    ofstream outfile;
    outfile.open(fileName.c_str());

    Mat img;
    stringstream msg;
  
    int thickness = -1;
    int lineType = 8;
    int radius   = 8;
    //w/32.0,

    // main loop 
    for(;;)
    {
        video >> img;
        
        if( img.empty() )
            break;
       
        msg.str("");
        msg     << (int)img.at<Vec3b>(pt1.x,pt1.y)[0] << " " 
                << (int)img.at<Vec3b>(pt1.x,pt1.y)[1] << " " 
                << (int)img.at<Vec3b>(pt1.x,pt1.y)[2];
        cout    << msg.str() << endl; 
        outfile << msg.str() << endl;

        circle( img,
                pt1,
                radius,
                Scalar( 0, 0, 255 ),
                thickness,
                lineType );

        imshow(title, img);
       
        if (cv::waitKey(delay)>=0)
            break;
    }

    outfile.close();

    return 0;
}

