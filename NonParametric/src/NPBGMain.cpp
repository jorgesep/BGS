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

//#include <iomanip> 
//#include <iostream>
//#include <fstream>
//#include <dirent.h>
//#include <sstream>
//#include <map>
//
//#include "bgs.h"
//#include "mdgkt_filter.h"
//#include "background_subtraction.h"
#include "Performance.h"
#include "utils.h"
#include "NPBGSubtractor.h"

using namespace cv;
using namespace std;
using namespace bgs;


const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ g | gtruth  |       | Input ground-truth directory }"
    "{ c | config  |       | Load init config file }"
    "{ n | frame   | 0     | Shift ground-truth in +/- n frames, e.g -n -3 or -n 3}"
    "{ p | point   |       | Print out RGB values of point,  e.g -p 250,300 }"
    "{ m | mask    | true  | Save foreground masks to foreground directory}"
    "{ v | verbose | false | Print out output messages by console}"
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
        cout << "./npbgs -i dir_jpeg/ -g ground_truth/ -c config/init.txt -s   " << endl << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Read input parameters
    const string inputVideoName  = cmd.get<string>("input");
    const string groundTruthName = cmd.get<string>("gtruth");
    const string initConfigName  = cmd.get<string>("config");
    const string displayPoint    = cmd.get<string>("point");
    int shiftFrame         = cmd.get<int>("frame");
    const bool saveMask          = cmd.get<bool>("mask");
    bool verbose                 = cmd.get<bool>("verbose");
    const bool displayImages     = cmd.get<bool>("show");

    //declaration of local variables.
    Performance perf;
    stringstream msg,ptmsg;
    ofstream outfile;
    int gt_cnt = 0;
    
    
    if (inputVideoName.empty()) {
        cout << "Insert video name" << endl;
        cmd.printParams();
        return -1;
    }
    
    //Input name could be either a video or directory of jpeg files  
    bool processing_video = false;
    int im_size = -1;
    map<unsigned int, string> im_files;

    if (!DirectoryExists(inputVideoName.c_str())) {
        processing_video = true;
    }
    else {
        // Read files from input directory
        list_files(inputVideoName,im_files, ".jpg");
        im_size = im_files.size();
            
        if (im_size == -1 || im_size == 0) {
            cout << "Not valid ground images directory ... " << endl;
            return -1;
        }
    }    

    //Create foreground mask directory
    if (saveMask) {
        CreateDirectory("npoutput");
        CreateDirectory("npoutput/background");
        CreateDirectory("npoutput/foregorund");
    }
    

    //Load initialization parameters
    NPBGConfig *config = new NPBGConfig;
    if (!initConfigName.empty()) {
        config = loadInitParametersFromFile(initConfigName);
    }

    //Check ground-truth
    bool compare = false;
    map<unsigned int, string>::iterator it;
    map<unsigned int, string> gt_files;
    int gt_size = -1;
    if (!groundTruthName.empty()) {
        compare=true;
        list_files(groundTruthName,gt_files);
        //outfile.open("output.txt");
        //outfile << bg_model.initParametersAsOneLineString() << endl;

        if (displayImages) {
            namedWindow("GROUND TRUTH", CV_WINDOW_NORMAL);
            moveWindow("GROUND TRUTH",400,300);
        }
        gt_size = gt_files.size();
        
        if (gt_size == 0) {
            cout << "Not valid ground truth directory ... " << endl;
            return -1;
        }
    }

    int delay = 25;
    VideoCapture video;
    Mat imframe;
    
    if (processing_video) {
        //create video object.
        video.open(inputVideoName);
        
        // Check video has been opened sucessfully
        if (!video.isOpened())
            return 0;
        
        delay         = 1000/video.get(CV_CAP_PROP_FPS);
        video >> imframe;
        //video.set(CV_CAP_PROP_POS_FRAMES,0);
        shiftFrame += 1;
        
    }
    else {
        //create video object.
        imframe = imread(im_files[1]);
        
        // Check file has been opened sucessfully
        if (imframe.data == NULL )
            return 0;
        
    }
    
    int cols  = imframe.cols;
    int rows  = imframe.rows;
    int matSize   = rows * cols;
    int frameType = imframe.type();
    int nchannels = CV_MAT_CN(frameType);


    //Get specific point to be displayed in the image.
    //Check input point to display value
    int nl=0,nc=0;
    Point pt(0,0);
    bool show_point = false;
    ofstream ptfile;
    if (!displayPoint.empty()) {
        pt = stringToPoint(displayPoint);
        nc = pt.x > cols  ? cols  : pt.x;
        nl = pt.y > rows ? rows : pt.y;
        show_point = true;

        //define outfile to save BGR pixel values
        stringstream name(""); 
        name << "pt_" << pt.x << "_" << pt.y << ".txt";
        ptfile.open(name.str().c_str());
    }
    
   
    // Create display windows 
    if (displayImages) { 
        namedWindow("image", CV_WINDOW_NORMAL);
        namedWindow("Mask", CV_WINDOW_NORMAL);
        namedWindow("foreground image", CV_WINDOW_NORMAL);
        moveWindow("image"           ,20,20);
        moveWindow("foreground image",20,300);
        moveWindow("foreground mask" ,400,20);
    } 
    
    
    // Initialize model
    // SequenceLength: number of samples for each pixel.
    // TimeWindowSize: Time window for sampling. for example in the call above, the bg will sample 50 points out of 100 frames.
    // this rate will affect how fast the model adapt.
    // SDEstimationFlag: True means to estimate suitable kernel bandcols to each pixel, False uses a default value.
    // lUseColorRatiosFlag: True means use normalized RGB for color (recommended.)
    NPBGSubtractor *BGModel = new NPBGSubtractor;
    
    unsigned int _length = config->SequenceLength;
    unsigned int _wsize  = config->TimeWindowSize;
    unsigned char stmflag= config->SDEstimationFlag;
    unsigned char ratioflag = config->UseColorRatiosFlag;
    
    BGModel->Intialize(rows, cols, nchannels, 50, 100, 1, 1);
    //BGModel->Intialize(rows, cols, nchannels, _length, _wsize, stmflag, ratioflag);
    //BGModel->Intialize(rows, cols, nchannels, config->SequenceLength, config->TimeWindowSize, config->SDEstimationFlag, config->UseColorRatiosFlag);

    //th: 0-1 is the probability threshold for a pixel to be a foregroud. 
    // typically make it small as 10e-8. the smaller the value the less false positive and more false negative.
    //alpha: 0-1, for color. typically set to 0.3. this affect shadow suppression.
    double th = config->Threshold1;
    double ap = config->Alpha;
    BGModel->SetThresholds(th,ap);
    
    unsigned int bgflag = 1;
    BGModel->SetUpdateFlag(bgflag);
    
    //Shift backward or forward ground truth sequence counter.
    //for compensating pre-processed frames in the filter.
    int cnt    = 0  + shiftFrame ; 

    Mat Frame;
    Mat gt_image;
    Mat ftimg;
    Mat Mask;
    Mask = Mat::zeros(rows,cols,CV_8UC1);
    //Mat FGImage;
    //Mat FGImage(1,matSize, CV_8U, Scalar::all(0));
    //Mat FGImage(rows,cols, CV_8UC1, Scalar::all(0));
    //FGImage = cv::Mat::zeros(rows,cols,CV_8UC1);
    Mat filteredFGImage(1,matSize, CV_8U, Scalar::all(0));

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
        //clean up all Mat structures.
        //This is done because, performance was improved.
        //FGImage         = Scalar::all(0);
        //filteredFGImage = Scalar::all(0);
        
        if (processing_video) {
            video >> Frame;
        }
        else
        {
            Frame = imread(im_files[cnt+1]);
        }

        if (Frame.empty())
        {    
            break;
        }
        
        if (cnt < config->FramesToLearn) {

            // add frame to the background
            BGModel->AddFrame(Frame.data);
            
            cnt +=1;
            
            continue;
        }
        
        // Build the background model with first N frames to learn
        if(cnt == config->FramesToLearn)
        {
            BGModel->Estimation();
        }




        //subtract the background from each new frame
        //Frame : new frame
        //FGImage : to pass out the resulted FG (must be allocated before call) size imagerows x imagecols unsigned char
        //FilteredFGImage : to pass out a filtered version of the FG. size imagerows x imagecols unsigned char
        //DisplayBuffers : a list of buffers for processing (Array of pointers to unsigned char buffers, each of size imagerows x imagecols)
        //((NPBGSubtractor *)BGModel)->NBBGSubtraction(Frame.data, FGImage.data, filteredFGImage.data, DisplayBuffers);
        //BGModel->NBBGSubtraction(Frame.data, FGImage.data, filteredFGImage.data, DisplayBuffers);
        ((NPBGSubtractor *)BGModel)->NBBGSubtraction(Frame.data, FGImage, FilterFGImage, DisplayBuffers);
        //((NPBGSubtractor *)BGModel)->NBBGSubtraction(Frame.data, FGImage.data, FilterFGImage, DisplayBuffers);

        
        //here you pass a mask where pixels with true value will be masked out of the update.
        //((NPBGSubtractor *)BGModel)->Update(FGImage.data);
        ((NPBGSubtractor *)BGModel)->Update(FGImage);

        Mask.data = FGImage;
        filteredFGImage.data = FilterFGImage;

        /*
        //save mask to local directory
        if (saveMask) {
            stringstream str;
            str << "npoutput/foreground/NPFG_" <<  cnt << ".jpg";
            
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            compression_params.push_back(100);
            imwrite(str.str(), FGImage, compression_params);
        }
        */
        
        /*
        //looking for ground truth file
        if ( cnt >=0 && compare && (it = gt_files.find(cnt)) != gt_files.end() ) {
            
            // open ground truth frame.
            gt_image = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);
            
            if( gt_image.data ) {
                
                //Compare both images
                perf.pixelLevelCompare(gt_image, FGImage);
                
                // display ground truth
                if (displayImages)
                    imshow("GROUND TRUTH", gt_image);
                
                //Print out debug messages to either file or std.
                msg.str("");
                msg     << cnt << " " << perf.asString() << " " ;
                if (show_point) 
                    msg << ptmsg.str();
                
                outfile << msg.str() << endl;
                
                if (verbose) 
                    cout    << msg.str() << endl; 
            }
            
            // counter of number of  processed frames
            gt_cnt++;
            
            

        }//end ground-truth compare
        */

        /*
        // Save pixel information in a local file
        if (show_point) {
            ptmsg.str("");
            ptmsg  
            << (int)Frame.at<Vec3f>(nl,nc)[0] << " " 
            << (int)Frame.at<Vec3f>(nl,nc)[1] << " " 
            << (int)Frame.at<Vec3f>(nl,nc)[2] ;
            ptfile << ptmsg.str() << endl;
        }
        */
        
        //Display sequence frames
        if (displayImages) {
            Frame.convertTo(ftimg, CV_8UC3);
            if (show_point){
                circle(ftimg,pt,8,Scalar(0,0,254),-1,8);
                //circle(FGImage,pt,8,Scalar(155,155,155),-1,8);
            }
            
            imshow("image", ftimg);
            imshow("Mask", Mask);
            //imshow("foreground image", filteredFGImage);

        }
        
        
        cnt++;
        
        char key=0;
        if (displayImages)
            key = (char)waitKey(delay);
        if( key == 27 ) { cout << "PRESSED BUTTON "<< endl; break;}
        
        /*
        //in case of not display option enabled stop execution 
        //after last ground-truth file was processed
        if (!displayImages && compare && gt_cnt >= gt_size)
            break;
        */

    }

    /*
    if (!groundTruthName.empty() && compare) {
        
        perf.calculateFinalPerformanceOfMetrics();
        
        cout    << perf.metricsStatisticsAsString() << endl;
        outfile << "# TPR FPR SPE MCC  TPR TNR SPE MCC" << endl;
        outfile << "# " << perf.metricsStatisticsAsString() << endl;
        outfile.close();
    }

    if (ptfile.is_open())
        ptfile.close();

    */    
    delete config;
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

