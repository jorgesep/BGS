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
#include "FrameReaderFactory.h"
#include "BGSTimer.h"

using namespace cv;
using namespace std;
using namespace boost::filesystem;
using namespace bgs;
using namespace seq;




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
    "{ p | point   |       | Show small red spot in frame image,  e.g -p 250,300 }"    
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
    const string savePoint       = cmd.get<string>("point");


    // local variables
    vector<string> im_files;
    
    
    // Verify input name is a video file or directory with image files.
    FrameReader *input_frame;
    try {
        input_frame = FrameReaderFactory::create_frame_reader(inputVideoName);
    } catch (...) {
        cout << "Invalid file name "<< endl;
        return 0;
    }
    
    int cols      = input_frame->getNumberCols(); 
    int rows      = input_frame->getNumberRows();
    int nchannels = input_frame->getNChannels();
    int delay     = input_frame->getFrameDelay();
    int matSize   = rows * cols;


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
    stringstream algorithm_params;
    if (saveMask) {
        
        // Create directoty if not exists and create a numbered internal directory.
        // np_mask/0, np_mask/1, ...
        create_foreground_directory(foreground_path);
                
        stringstream param;
        param << foreground_path << "/parameters.txt" ;

        algorithm_params 
        << "#"
        << " Alpha="          << Alpha 
        << " Threshold="      << Threshold 
        << " FramesToLearn="  << FramesToLearn 
        << " SequenceLength=" << SequenceLength 
        << " TimeWindowSize=" << TimeWindowSize ; 

        std::ofstream outfile;
        outfile.open(param.str().c_str());
        outfile << algorithm_params.str() ;
        outfile.close();
        
    }
    
    
    // Create display windows

    /// aspect ratio of pictures 5:4
    int width  = 200;
    int height = int(width/1.25);
    int border = 1;
    int sep    = 1;
    int ncol   = 2; 
    int nrow   = 1;
    int wsize  = border*2 + ncol*width  + (ncol-1)*sep;
    int hsize  = border*2 + nrow*height + (nrow-1)*sep;
    int m = border;
    int n = border;

    if (displayImages) {
        namedWindow("Non-Parametric", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
        moveWindow("Non-Parametric", 50, 50);
    }

    //Get specific point to be displayed in the image.
    //Check input point to display value
    std::ofstream point_file;
    int nl=0,nc=0;
    Point pt(0,0);
    if (!savePoint.empty()) {
        pt = stringToPoint(savePoint);
        nc = pt.x > cols  ? cols  : pt.x;
        nl = pt.y > rows ? rows : pt.y;
    
        //define outfile to save BGR pixel values
        stringstream name("");
        name << "pt_" << pt.x << "_" << pt.y << ".txt";
        point_file.open(name.str().c_str());
    }

    
    NPBGSubtractor *BGModel = new NPBGSubtractor;
    BGModel->Intialize(rows, cols, nchannels, SequenceLength, TimeWindowSize, SDEstimationFlag, UseColorRatiosFlag);
    BGModel->SetThresholds(Threshold,Alpha);
    BGModel->SetUpdateFlag(UpdateFlag);
    
    Mat Frame;
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

   
    //Shift backward or forward ground truth sequence counter.
    //for compensating pre-processed frames in the filter.
    int cnt    = 0 ; 
   

    // Start timer
    string description = "NP " + algorithm_params.str() ;
    BGSTimer::Instance()->setSequenceName(inputVideoName.c_str(), description) ;
    BGSTimer::Instance()->registerStartTime();

    // main loop 
    for(;;)
    {
        Frame = Scalar::all(0);
        input_frame->getFrame(Frame);
        if (Frame.empty()) break;
        
        cnt = input_frame->getFrameCounter();      
        
        if (cnt < FramesToLearn) {

            // add frame to the background
            BGModel->AddFrame(Frame.data);
            
            
            continue;
        }
        
        // Build the background model with first N frames to learn
        if( cnt == FramesToLearn )
            BGModel->Estimation();



        Mask = Scalar::all(0);
        FilteredFGImage = Scalar::all(0);

        //subtract the background from each new frame
        ((NPBGSubtractor *)BGModel)->NBBGSubtraction(Frame.data, Mask.data, FilteredFGImage.data, DisplayBuffers);

        
        //here you pass a mask where pixels with true value will be masked out of the update.
        ((NPBGSubtractor *)BGModel)->Update(Mask.data);

        //Mask.data = FGImage;

        // Applying morphological filter
        // Erode the image
        Mat Eroded; // the destination image
        if (ApplyMorphologicalFilter) {
            Mat Element(2,2,CV_8U,cv::Scalar(1));
            erode(Mask,Eroded,Element);
            Mask = Scalar::all(0);
            Eroded.copyTo(Mask);
        }

        if (saveMask &&  cnt >= InitFGMaskFrame &&  cnt <= EndFGMaskFrame) {
            stringstream str;
            str << foreground_path << "/" <<  cnt << ".png";
            
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(9);

            try {
                imwrite(str.str(), Mask, compression_params);
            }
            catch (runtime_error& ex) {
                cout << "Exception converting image to PNG format: " << ex.what() << endl;
            }
            catch (...) {
                cout << "Unknown Exception converting image to PNG format: " << endl;
            }
        }



        // Save pixel information in a local file
        if (!savePoint.empty()) {
            Mat pointImg;
            Frame.convertTo(pointImg,CV_8UC3);
            stringstream msgPoint;

            msgPoint  << (int)pointImg.at<Vec3b>(nl,nc)[0] << " "
                      << (int)pointImg.at<Vec3b>(nl,nc)[1] << " "
                      << (int)pointImg.at<Vec3b>(nl,nc)[2] ;
            point_file<< msgPoint.str() << endl;
        }


        if (displayImages) {

            Frame.convertTo(ftimg, CV_8UC3);

            if (!savePoint.empty()){ 
                circle(ftimg,pt,8,Scalar(0,0,254),-1,8);
                circle(Mask,pt,8,Scalar(0,0,254),-1,8);
            }

            m = border;
            n = border;

            Mat fgimg;
            fgimg.create(Size(wsize,hsize), CV_8UC3) ;
            //fgimg = Scalar::all(255);

            // Invert color of Mask from black to white
            Mat InvertedMask;
            threshold(Mask, InvertedMask, 200, 255, 1);

            Mat ColorMask;
            cvtColor(InvertedMask,ColorMask, CV_GRAY2BGR);


            Mat ImageROI   = fgimg(Rect( m, n, width, height ));
            resize(ftimg, ImageROI  , ImageROI.size()  , 0, 0 );

            ImageROI = fgimg(Rect( m + sep + width, n, width, height ));
            resize(ColorMask , ImageROI, ImageROI.size(), 0, 0 );

            imshow("Non-Parametric", fgimg);

            char key=0;
            key = (char)waitKey(delay);
            if( key == 27 ) 
                break;

            // pause program in with space key
            if ( key == 32) {
                bool pause = true;
                while (pause)
                {
                    key = (char)waitKey(delay);
                    if (key == 32)
                        pause = false;
                    // save frame with return key
                    if (key == 13) {
                        stringstream str;
                        str << "np_" << cnt << "_bg.png" ;
                        imwrite( str.str()  , ftimg  );
                        str.str("") ;
                        str << "np_" << cnt << "_fg.png" ;
                        imwrite( str.str(), Mask );
                        str.str("") ;
                        str << "np_" << cnt << "_bg_fg.png" ;
                        imwrite( str.str(), fgimg );
                    }
                }
            }
        }
        
        if (!displayImages && cnt > EndFGMaskFrame)
            break;
            
        
        
        
    }

    // Return elapsed time and sve it in file.
    BGSTimer::Instance()->registerStopTime();
    cout << BGSTimer::Instance()->getSequenceElapsedTime() << endl;
    BGSTimer::deleteInstance();

    delete BGModel;
    //delete FilterFGImage;
    //delete FGImage;

    delete [] DisplayBuffers[0];   // First delete pointer content
    delete [] DisplayBuffers[1];
    delete [] DisplayBuffers[2];
    delete [] DisplayBuffers[3];
    delete [] DisplayBuffers[4];
    delete [] DisplayBuffers;

    delete input_frame;

    if (point_file.is_open())
        point_file.close();

    return 0;
}

