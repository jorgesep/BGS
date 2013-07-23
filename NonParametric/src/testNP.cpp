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

//#include "Performance.h"
//#include "utils.h"
#include "NPBGSubtractor.h"

using namespace cv;
using namespace std;
//using namespace bgs;


const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ h | help    | false | Print help message }"
};



int main( int argc, char** argv )
{
    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);

    if (cmd.get<bool>("help"))
    {
        cout << "Non-Parametric Model for background subtraction Program." << endl;
        cout << "------------------------------------------------------------" << endl;
        cout << "Process input video comparing with its ground truth.        " << endl;
        cout << "OpenCV Version : "  << CV_VERSION << endl;
        cout << "Example:                                                    " << endl;
        cout << "./npbgs -i dir_jpeg/ -g ground_truth/ -c config/init.txt -s   " << endl << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Read input parameters
    const string inputVideoName  = cmd.get<string>("input");
    
    if (inputVideoName.empty()) {
        cout << "Insert video name" << endl;
        cmd.printParams();
        return -1;
    }
    
    
    VideoCapture video;
    video.open(inputVideoName);
    // Check video has been opened sucessfully
    if (!video.isOpened())
        return 0;

    //Input name could be either a video or directory of jpeg files  
    int delay = 1000/video.get(CV_CAP_PROP_FPS);
    int cols = video.get(CV_CAP_PROP_FRAME_WIDTH);
    int rows = video.get(CV_CAP_PROP_FRAME_HEIGHT);
    int matSize   = rows * cols;
    int nchannels = 3;

    
    NPBGSubtractor *BGModel = new NPBGSubtractor;
    BGModel->Intialize(rows, cols, nchannels, 50, 100, 1, 1);
    BGModel->SetThresholds(1e-7,0.3);
    BGModel->SetUpdateFlag(1);
    
    //Shift backward or forward ground truth sequence counter.
    //for compensating pre-processed frames in the filter.
    int cnt    = 0 ; 

    Mat Frame;
    Mat ftimg;
    Mat Mask(rows,cols,CV_8UC1,Scalar::all(0));
    //Mask = Mat::zeros(rows,cols,CV_8UC1);

    // For testing allocating memory
    unsigned char **DisplayBuffers;
    DisplayBuffers = new unsigned char*[5];
    DisplayBuffers[0] = new unsigned char[cols*rows];
    DisplayBuffers[1] = new unsigned char[cols*rows];
    DisplayBuffers[2] = new unsigned char[cols*rows];
    DisplayBuffers[3] = new unsigned char[cols*rows];
    DisplayBuffers[4] = new unsigned char[cols*rows];

    unsigned char *FilterFGImage = new unsigned char[cols*rows];
    unsigned char *FGImage = new unsigned char[cols*rows];

    // main loop 
    for(;;)
    {
       
        video >> Frame;
        if (Frame.empty()) break;
        
        if (cnt < 10) {

            // add frame to the background
            BGModel->AddFrame(Frame.data);
            
            cnt +=1;
            
            continue;
        }
        
        // Build the background model with first N frames to learn
        if( cnt == 10 )
            BGModel->Estimation();



        //subtract the background from each new frame
        ((NPBGSubtractor *)BGModel)->NBBGSubtraction(Frame.data, FGImage, FilterFGImage, DisplayBuffers);

        
        //here you pass a mask where pixels with true value will be masked out of the update.
        ((NPBGSubtractor *)BGModel)->Update(FGImage);

        Mask.data = FGImage;

        Frame.convertTo(ftimg, CV_8UC3);
        imshow("image", ftimg);
        imshow("Mask", Mask);
        
        
        
        cnt++;
        
        char key=0;
        key = (char)waitKey(delay);
        if( key == 27 ) 
            break;
        
    }

    delete BGModel;
    delete FilterFGImage;
    delete FGImage;

    delete [] DisplayBuffers[0];   // First delete pointer content
    delete [] DisplayBuffers[1];
    delete [] DisplayBuffers[2];
    delete [] DisplayBuffers[3];
    delete [] DisplayBuffers[4];
    delete [] DisplayBuffers;

    return 0;
}

