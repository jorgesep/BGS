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
#include "UCVBuilder.h"
#include "FrameReaderFactory.h"

//#include "Performance.h"
#include "utils.h"
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
    const string inputName        = cmd.get<string>("input");
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
        return 0;
    }
    
    // Checking out config file available
    string configfile = "config/Framework.xml";
    
    if ( !exists(configfile) && !is_regular_file(configfile) ) {
        cout << "File " << configfile << " not found." << endl;
        return 0;
    }
        
    // Load configuration file
    FileStorage fs(configfile, FileStorage::READ);
    
    bool algorithmMOG2Enabled     = (int)fs["MOG2"];
    bool algorithmNPEnabled       = (int)fs["NP"];
    bool algorithmSAGMMEnabled    = (int)fs["SAGMM"];
    bool algorithmUCVEnabled    = (int)fs["UCV"];
    int  InitFGMaskFrame          = (int)fs["InitFGMaskFrame"];
    int  EndFGMaskFrame           = (int)fs["EndFGMaskFrame"];
    
    fs.release();
    
    
    // Create display windows
    if (displayImages) {
        namedWindow("Current Frame", CV_WINDOW_NORMAL);
        moveWindow("Current Frame", 20, 20);

        if (algorithmMOG2Enabled){
            namedWindow("MOG2", CV_WINDOW_NORMAL);
            moveWindow("MOG2", 300, 20);
        }
        
        if (algorithmNPEnabled) {
            namedWindow("Non Parametric", CV_WINDOW_NORMAL);
            moveWindow("Non Parametric", 200, 20);
        }
        
        if (algorithmSAGMMEnabled) {
            namedWindow("SAGMM", CV_WINDOW_NORMAL);
            moveWindow("SAGMM", 200, 200);
        }

        if (algorithmUCVEnabled) {
            namedWindow("UCV", CV_WINDOW_NORMAL);
            moveWindow("UCV", 200, 200);
        }
    }
    

    // Instances of algorithms 

    int col   = input_frame->getNumberCols(); 
    int row   = input_frame->getNumberRows();
    int nch   = input_frame->getNChannels();
    int delay = input_frame->getFrameDelay();
    
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
    
    Framework *ucv = new Framework();
    ucv->setAlgorithm(new UCVBuilder(col,row,nch));
    ucv->setName("UCV");
    ucv->loadConfigParameters();
    ucv->initializeAlgorithm();
    
    
    // Prapare directory masks
    string np_foreground_path    = "np_mask";
    string sagmm_foreground_path = "sagmm_mask";
    string mog2_foreground_path  = "mog2_mask";
    string ucv_foreground_path  = "ucv_mask";
    
    if (saveForegroundMask) {

        // Create directoty if not exists and create a numbered internal directory.
        // np_mask/0, np_mask/1, ...
        if (algorithmNPEnabled) 
            bgs::create_foreground_directory(np_foreground_path);
        if (algorithmSAGMMEnabled) 
            bgs::create_foreground_directory(sagmm_foreground_path);
        if (algorithmMOG2Enabled)
            bgs::create_foreground_directory(mog2_foreground_path);
        if (algorithmUCVEnabled)
            bgs::create_foreground_directory(ucv_foreground_path);


        ofstream outfile;
        stringstream param;
        if (algorithmNPEnabled) {
            param << np_foreground_path << "/parameters.txt" ;
            outfile.open(param.str().c_str());
            outfile << np->getConfigurationParameters();
            outfile.close();
        }
        
        param.str("");
        if (algorithmSAGMMEnabled) {
            param << sagmm_foreground_path << "/parameters.txt" ;
            outfile.open(param.str().c_str());
            outfile << sagmm->getConfigurationParameters();
            outfile.close();
        }

        param.str("");
        if (algorithmMOG2Enabled){
            param << mog2_foreground_path << "/parameters.txt" ;
            outfile.open(param.str().c_str());
            outfile << mog2->getConfigurationParameters();
            outfile.close();
        }

        param.str("");
        if (algorithmUCVEnabled){
            param << ucv_foreground_path << "/parameters.txt" ;
            outfile.open(param.str().c_str());
            outfile << ucv->getConfigurationParameters();
            outfile.close();
        }
    }
    
   
     
    Mat Frame;
    Mat Image;
    Mat Foreground;
    Mat NPMask;
    Mat SAMask;
    Mat UCVMask;
   
    int cnt = 0;

    // main loop
    for(;;)
    {

        NPMask     = Scalar::all(0);
        Foreground = Scalar::all(0);
        SAMask     = Scalar::all(0);
        UCVMask    = Scalar::all(0);

        input_frame->getFrame(Frame);
        
        if (Frame.empty()) break;
    
        if (algorithmMOG2Enabled) {
            mog2->updateAlgorithm(Frame, Foreground);
            
            if (displayImages)
                imshow("MOG2", Foreground);
        }
        
        if (algorithmNPEnabled) {
            np->updateAlgorithm(Frame, NPMask);
            
            if (displayImages)
                imshow("Non Parametric", NPMask);
        }
        
        if (algorithmSAGMMEnabled) {
            sagmm->updateAlgorithm(Frame, SAMask);
            
            if (displayImages)
                imshow("SAGMM", SAMask);
        }
        
        if (algorithmUCVEnabled) {
            ucv->updateAlgorithm(Frame, UCVMask);
            
            if (displayImages)
                imshow("UCV", UCVMask);
        }
         // Save foreground images
        if (saveForegroundMask && cnt >= InitFGMaskFrame && cnt <= EndFGMaskFrame) {
            stringstream str;
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(9);

            try {
                if (algorithmMOG2Enabled) {
                    str << mog2_foreground_path << "/" <<  cnt << ".png";
                    imwrite(str.str(), Foreground, compression_params);
                }
                if (algorithmNPEnabled) {
                    str.str("");
                    str << np_foreground_path << "/" <<  cnt << ".png";
                    imwrite(str.str(), NPMask, compression_params);
                    
                }
                if (algorithmSAGMMEnabled) {
                    str << sagmm_foreground_path << "/" <<  cnt << ".png";
                    imwrite(str.str(), SAMask, compression_params);
                    
                }
                if (algorithmUCVEnabled) {
                    str << ucv_foreground_path << "/" <<  cnt << ".png";
                    imwrite(str.str(), UCVMask, compression_params);
                    
                }
            }
            catch (runtime_error& ex) {
                cout << "Exception converting image to PNG format: " << ex.what() << endl;
            }
            catch (...) {
                cout << "Unknown Exception converting image to PNG format: " << endl;
            }


            
        }
        

        
        
        if (displayImages) {
            Frame.convertTo(Image, CV_8UC3);
            imshow("Current Frame", Image);
            
            char key=0;
            key = (char)waitKey(delay);
            if( key == 27 )
                break;
        }

        cnt +=1;
        
    }
    
    
    delete mog2;
    delete ucv;
    delete np;
    delete input_frame;


    return 0;
}

