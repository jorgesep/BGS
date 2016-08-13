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

//#include "bgs.h"
//#include "mdgkt_filter.h"
//#include "background_subtraction.h"
//#include "Performance.h"
#include "utils.h"
#include "FrameReaderFactory.h"


#include "icdm_model.h"

using namespace cv;
using namespace std;
using namespace bgs;
using namespace seq;


const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ r | read    |       | Read background model from  file }"
    "{ w | write   |       | Write background model in a file }"
    "{ p | point   |       | Print out RGB values of point,  e.g -p 250,300 }"
    "{ s | show    | true  | Show images of video sequence }"
    "{ h | help    | false | Print help message }"
};


void display_message()
{
    cout << "Play any video sequence.                                    " << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "                                                            " << endl;
    cout << "OpenCV Version : "  << CV_VERSION << endl;
    cout << "Example:                                                    " << endl;
    cout << "./play_sequence -i movie_file                               " << endl << endl;
    cout << "------------------------------------------------------------" << endl <<endl;
}

int get_video_codec_type(string source)
{
    VideoCapture inputVideo(source);              // Open input
    if (!inputVideo.isOpened())
    {
        cout  << "Could not open the input video: " << source << endl;
        return -1;
    }



    int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));// Get Codec Type- Int form
    // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    
    Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
         << " of nr#: " << inputVideo.get(CV_CAP_PROP_FRAME_COUNT) << endl;
    cout << "Input codec type: " << EXT << endl;

    inputVideo.release();

    return 1;
}




int main( int argc, char** argv )
{

    //declaration of local variables.
    stringstream msg,ptmsg;
    ofstream outfile, ptfile;
    bool show_point = false;

    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);


    // Read input parameters
    const string inputName       = cmd.get<string>("input");
    const string bgModelName     = cmd.get<string>("read");
    const string saveName        = cmd.get<string>("write");
    const string displayPoint    = cmd.get<string>("point");
    const bool displayImages     = cmd.get<bool>("show");

    

    if (cmd.get<bool>("help")) {
       display_message();
       cmd.printParams();
       return 0;
    }
    
    // Verify input name is a video file or directory with image files.
    FrameReader *input_frame;
    try {
        input_frame = FrameReaderFactory::create_frame_reader(inputName);
    } catch (...) {
        cout << "Invalid file name "<< endl;
        return 0;
    }

    get_video_codec_type(inputName);

    // Instances of algorithms
    int col   = input_frame->getNumberCols();
    int row   = input_frame->getNumberRows();
    //int nch   = input_frame->getNChannels();
    int delay = input_frame->getFrameDelay();


    // Create display windows
    if (displayImages) {
        namedWindow("Current Frame", CV_WINDOW_NORMAL);
        moveWindow("Current Frame", 20, 20);
    }

    //Get specific point to be displayed in the image.
    //Check input point to display value
    int nl=0,nc=0;
    Point pt(0,0);
    if (!displayPoint.empty()) {
        pt = stringToPoint(displayPoint);
        nc = pt.x > col  ? col : pt.x;
        nl = pt.y > row  ? row : pt.y;
        show_point = true;
    
        //define outfile to save BGR pixel values
        stringstream name("");
        name << "pt_" << pt.x << "_" << pt.y << ".txt";
        ptfile.open(name.str().c_str());
    }



    Mat Frame;
    Mat Image;
    int cnt = 0;

    // main loop
    for(;;)
    {
        input_frame->getFrame(Frame);

        if (Frame.empty()) break;

        Frame.convertTo(Image, CV_8UC3);

        // Save pixel information in a local file
        if (show_point) {
            ptmsg.str("");
            ptmsg  
                   << (int)Image.at<Vec3b>(nl,nc)[0] << " "
                   << (int)Image.at<Vec3b>(nl,nc)[1] << " "
                   << (int)Image.at<Vec3b>(nl,nc)[2] << " "

                   << (int)Frame.at<Vec3b>(nl,nc)[0] << " "
                   << (int)Frame.at<Vec3b>(nl,nc)[1] << " "
                   << (int)Frame.at<Vec3b>(nl,nc)[2] ;

            ptfile << ptmsg.str() << endl;

            circle(Image,pt,8,Scalar(0,0,254),-1,8);
        }

        imshow("Current Frame", Image);

        char key=0;
        key = (char)waitKey(delay);
        if( key == 27 )
            break;

        cnt +=1;
    }

    delete input_frame;

    return 0;

}

