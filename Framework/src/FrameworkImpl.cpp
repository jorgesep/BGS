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

#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include <fstream>

#include "BGSFramework.h"
#include "MOGBuilder.h"
#include "MOG2Builder.h"
#include "NPBuilder.h"
#include "SAGMMBuilder.h"
#include "FrameReaderFactory.h"

//#include "Performance.h"
//#include "utils.h"
//#include "NPBGSubtractor.h"
//#include "mdgkt_filter.h"

using namespace cv;
using namespace std;
using namespace boost::filesystem;
using namespace seq;
//using namespace bgs;




const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ m | mask    | true  | Save foreground masks}"
    "{ s | show    | true  | Show frame sequence}"
    "{ h | help    | false | Print help message }"
};

void display_message()
{
    cout << "Background Subtraction Evaluation Framework.                " << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "Process input video comparing with its ground truth.        " << endl;
    cout << "OpenCV Version : "  << CV_VERSION << endl;
    cout << "Example:                                                    " << endl;
    cout << "./bgs_framework -i movie_file                               " << endl << endl;
    cout << "------------------------------------------------------------" << endl <<endl;
}



int main( int argc, char** argv )
{
    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);

    // Reading input parameters
    const string inputName   = cmd.get<string>("input");
    const bool displayImages      = cmd.get<bool>("show");
    const bool saveForegroundMask = cmd.get<bool>("mask");
    

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
        return -1;
    }
    
    // Create display windows
    if (displayImages) {
        namedWindow("Current Frame", CV_WINDOW_NORMAL);
        moveWindow("Current Frame", 20, 20);

        namedWindow("MOG2", CV_WINDOW_NORMAL);
        moveWindow("MOG2", 300, 20);

        namedWindow("Non Parametric", CV_WINDOW_NORMAL);
        moveWindow("Non Parametric", 200, 20);

        namedWindow("SAGMM", CV_WINDOW_NORMAL);
        moveWindow("SAGMM", 200, 200);
    }
    

    int col = input_frame->getNumberCols(); 
    int row = input_frame->getNumberRows();
    int nch = input_frame->getNChannels();
    int delay = input_frame->getFrameDelay();
    
    // Creates algorithm objects    
    Framework* mog2 = new Framework();
    mog2->setAlgorithm(new MOG2Builder());
    mog2->setName("MOG2");
    mog2->loadConfigParameters();
    mog2->initializeAlgorithm();
    

    Framework* np = new Framework();
    np->setAlgorithm(new NPBuilder(col,row,nch));
    np->setName("NP");
    np->loadConfigParameters();
    np->initializeAlgorithm();
   
    Framework *sagmm = new Framework();
    sagmm->setAlgorithm(new SAGMMBuilder());
    sagmm->setName("SAGMM");
    sagmm->loadConfigParameters();
    sagmm->initializeAlgorithm();
    
     
    Mat Frame;
    Mat Image;
    Mat Foreground;
    Mat NPMask;
    Mat SAMask;
   
    int cnt = 0;

    // main loop
    for(;;)
    {

        NPMask     = Scalar::all(0);
        Foreground = Scalar::all(0);
        SAMask     = Scalar::all(0);

        input_frame->getFrame(Frame);
        
        if (Frame.empty()) break;
    
        mog2->updateAlgorithm(Frame, Foreground);
        np->updateAlgorithm(Frame, NPMask);
        sagmm->updateAlgorithm(Frame, SAMask);
        
        if (displayImages) {
            Frame.convertTo(Image, CV_8UC3);
            imshow("Current Frame", Image);
            imshow("MOG2", Foreground);
            imshow("Non Parametric", NPMask);
            imshow("SAGMM", SAMask);
            
            char key=0;
            key = (char)waitKey(delay);
            if( key == 27 )
                break;
        }

        cnt +=1;
        
    }
    
    
    delete mog2;
    delete np;
    delete input_frame;


    return 0;
}

