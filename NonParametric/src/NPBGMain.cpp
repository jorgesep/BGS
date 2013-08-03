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



//#include "Performance.h"
#include "utils.h"
#include "NPBGSubtractor.h"
#include "mdgkt_filter.h"

using namespace cv;
using namespace std;
using namespace boost::filesystem;
using namespace bgs;




void rename_dir(string name)
{
    unsigned long dir_count = 0;
    directory_iterator end_iter;
    for ( directory_iterator dir_itr( "." ); dir_itr != end_iter;  ++dir_itr ) {
        if ( is_directory( *dir_itr ) && dir_itr->path().filename().stem() == name)
            dir_count++;

    }
    if (dir_count > 0) {
        stringstream p;
        p << name << "." << dir_count;
        rename(name, p.str());
        create_directory(name);
    }
}

const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ m | mask    | true  | Save foreground masks}"
    "{ s | show    | false | Show images of video sequence }"
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
        cout << "./npbgs -i dir_jpeg/  -s                  " << endl << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Reading input parameters
    const string inputVideoName  = cmd.get<string>("input");
    const bool displayImages     = cmd.get<bool>("show");
    const bool saveMask          = cmd.get<bool>("mask");
    const bool preProcessingFilter      = cmd.get<bool>("pre");


    // local variables
    int delay     = 25;
    int cols      = 0;
    int rows      = 0;
    int matSize   = 0;
    int nchannels = 3;
    bool processing_video = false;
    int im_size = -1;
    vector<string> im_files;
    
    // Verify input name is a video file or sequences of jpg files
    path _path (inputVideoName.c_str());
    
    if (is_directory(_path)) {
        
        vector<path> v;
        copy(directory_iterator(_path), directory_iterator(), back_inserter(v));

        
        for (vector<path>::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it) {
            if (it->extension() == ".jpg")
                im_files.push_back((canonical(*it).string()));
        }

        sort(im_files.begin(), im_files.end());
        
        im_size = im_files.size();
        if (im_size == -1 || im_size == 0) {
            cout << "Not valid images directory ... " << endl;
            return -1;
        }

        //create video object.
        Mat Frame = imread(im_files[0]);
        
        // Check file has been opened sucessfully
        if (Frame.data == NULL )
            return 0;
        
        cols  = Frame.cols;
        rows = Frame.rows;
        int frameType = Frame.type();
        nchannels = CV_MAT_CN(frameType);        

    }
    else if (is_regular_file(_path)) {
        VideoCapture video(inputVideoName.c_str());
        if (!video.isOpened()) {
            cout << inputVideoName << " Not video file" << endl;
            return -1;
        }
        
        Mat Frame;
        video >> Frame;

        delay = 1000/video.get(CV_CAP_PROP_FPS);
        cols = video.get(CV_CAP_PROP_FRAME_WIDTH);
        rows = video.get(CV_CAP_PROP_FRAME_HEIGHT);
        int frameType = Frame.type();        
        nchannels = CV_MAT_CN(frameType);
        video.release();
        
        processing_video = true;

    }
    else {
        cout << "Insert either directory or video name" << endl;
        cmd.printParams();
        return -1;
    }
    matSize   = rows * cols;

    // Read algorithm parameters    
    //Load initialization parameters
    string config_filename = "config/np.xml";
    path config_path(config_filename);
    if (!is_regular_file(config_path)) {
        cout << "Configuration file " << config_filename << " not found! " << endl;
        return -1;
    }
    
    
    FileStorage fs(config_filename, FileStorage::READ);
    int FramesToLearn                = (int)fs["FramesToLearn"];
    int SequenceLength               = (int)fs["SequenceLength"];
    int TimeWindowSize               = (int)fs["TimeWindowSize"];
    unsigned char UpdateFlag         = (int)fs["UpdateFlag"];
    unsigned char SDEstimationFlag   = (int)fs["SDEstimationFlag"];
    unsigned char UseColorRatiosFlag = (int)fs["UseColorRatiosFlag"];
    double Threshold                 = (double)fs["Threshold"];
    double Alpha                     = (double)fs["Alpha"];
    int InitFGMaskFrame              = (int)fs["InitFGMaskFrame"];
    int EndFGMaskFrame               = (int)fs["EndFGMaskFrame"];
    int ApplyMorphologicalFilter     = (int)fs["ApplyMorphologicalFilter"];
    fs.release();
    

    // Create mask directory
    string foreground_path = "np_mask";
    if (saveMask) {
        
        // Create directoty if not exists and create a numbered internal directory.
        // np_mask/0, np_mask/1, ...
        create_foreground_directory(foreground_path);
                
        ofstream outfile;
        stringstream param;
        param << foreground_path << "/parameters.txt" ;
        outfile.open(param.str().c_str());
        outfile 
        << "#"
        << " Alpha=" << Alpha 
        << " Threshold=" << Threshold 
        << " FramesToLearn=" << FramesToLearn 
        << " SequenceLength=" << SequenceLength 
        << " TimeWindowSize=" << TimeWindowSize ; 
        outfile.close();
        
    }
    
    
    // Create display windows
    if (displayImages) {
        namedWindow("Image", CV_WINDOW_NORMAL);
        namedWindow("Mask", CV_WINDOW_NORMAL);
        moveWindow("Image", 20, 20);
        moveWindow("Mask" , 20, 300);
    }
    
    NPBGSubtractor *BGModel = new NPBGSubtractor;
    BGModel->Intialize(rows, cols, nchannels, SequenceLength, TimeWindowSize, SDEstimationFlag, UseColorRatiosFlag);
    BGModel->SetThresholds(Threshold,Alpha);
    BGModel->SetUpdateFlag(UpdateFlag);
    
    Mat Frame;
    Mat FilteredFrame;
    Mat ftimg;
    Mat Mask(rows,cols,CV_8UC1,Scalar::all(0));
    Mat FilteredFGImage(rows,cols,CV_8UC1,Scalar::all(0));
    Mat tmp(rows,cols,CV_8UC1,Scalar::all(0));

    // For testing allocating memory
    unsigned char **DisplayBuffers;

    DisplayBuffers = new unsigned char*[5];
    DisplayBuffers[0] = new unsigned char[matSize];
    DisplayBuffers[1] = new unsigned char[matSize];
    DisplayBuffers[2] = new unsigned char[matSize];
    DisplayBuffers[3] = new unsigned char[matSize];
    DisplayBuffers[4] = new unsigned char[matSize];

    
    //unsigned char *FilterFGImage = new unsigned char[cols*rows];
    //unsigned char *FGImage = new unsigned char[cols*rows];

    VideoCapture video;
    if (processing_video) {
        video.open(inputVideoName);
        // Check video has been opened sucessfully
        if (!video.isOpened())
            return 0;
    }
    
    //Shift backward or forward ground truth sequence counter.
    //for compensating pre-processed frames in the filter.
    unsigned int cnt    = 0 ; 
    
    

    // main loop 
    for(;;)
    {
        if (processing_video)
            video >> Frame;
        else
            Frame = imread(im_files[cnt+1]);
        
        if (Frame.empty()) break;
              
        if (cnt < 10) {

            // add frame to the background
            if (preProcessingFilter)
                BGModel->AddFrame(FilteredFrame.data);
            else
                BGModel->AddFrame(Frame.data);
            
            cnt +=1;
            
            continue;
        }
        
        // Build the background model with first N frames to learn
        if( cnt == 10 )
            BGModel->Estimation();



        Mask = Scalar::all(0);
        FilteredFGImage = Scalar::all(0);

        //subtract the background from each new frame
        if (preProcessingFilter)
            ((NPBGSubtractor *)BGModel)->NBBGSubtraction(FilteredFrame.data, Mask.data, FilteredFGImage.data, DisplayBuffers);
        else
            ((NPBGSubtractor *)BGModel)->NBBGSubtraction(Frame.data, Mask.data, FilteredFGImage.data, DisplayBuffers);

        
        //here you pass a mask where pixels with true value will be masked out of the update.
        ((NPBGSubtractor *)BGModel)->Update(Mask.data);

        //Mask.data = FGImage;

        // Applying morphological filter
        // Erode the image
        Mat Eroded; // the destination image
        if (ApplyMorphologicalFilter) {
            Mat Element(2,2,CV_8U,cv::Scalar(1));
            //erode(Mask,Eroded,Mat());
            erode(Mask,Eroded,Element);
        }
        else {
            Mask.copyTo(Eroded);
        }

        if (saveMask && cnt >= InitFGMaskFrame && cnt <= EndFGMaskFrame) {
            stringstream str;
            str << foreground_path << "/" <<  cnt << ".jpg";
            
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            compression_params.push_back(100);
            //imwrite(str.str(), Mask, compression_params);
            imwrite(str.str(), Eroded, compression_params);
        }

        if (displayImages) {
            Frame.convertTo(ftimg, CV_8UC3);
            imshow("Image", ftimg);
            //imshow("Mask", Mask);
            imshow("Mask", Eroded);

            char key=0;
            key = (char)waitKey(delay);
            if( key == 27 ) 
                break;
        }
        
        if (!displayImages && cnt > EndFGMaskFrame)
            break;
            
        
        
        
        cnt++;
        
        
    }

    delete BGModel;
    //delete FilterFGImage;
    //delete FGImage;

    delete [] DisplayBuffers[0];   // First delete pointer content
    delete [] DisplayBuffers[1];
    delete [] DisplayBuffers[2];
    delete [] DisplayBuffers[3];
    delete [] DisplayBuffers[4];
    delete [] DisplayBuffers;

    return 0;
}

