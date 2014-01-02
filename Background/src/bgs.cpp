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

#include "bgs.h"
#include "mdgkt_filter.h"
#include "background_subtraction.h"
#include "Performance.h"
#include "utils.h"

#include "icdm_model.h"

using namespace cv;
using namespace std;
using namespace bgs;


const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ o | output  |       | Output video }"
    "{ g | gtruth  |       | Input ground-truth directory }"
    "{ r | read    |       | Read background model from  file }"
    "{ w | write   |       | Write background model in a file }"
    "{ n | frame   | 0     | Shift ground-truth in +/- n frames, e.g -n -3 or -n 3}"
    "{ f | filter  | true  | Apply smooth preprocessing filter, Default true.}"
    "{ p | point   |       | Print out RGB values of point,  e.g -p 250,300 }"
    "{ l | range   |       | Frame range of the mask to be saved,  e.g -l 216,682 }"
    "{ m | mask    | true  | Save foreground masks to fgmask directory}"
    "{ v | verbose | false | Print out output messages by console}"
    "{ d | debug   | false | Debug mode, print by console internal gaussian parameters }"
    "{ s | show    | false | Show images of video sequence }"
    "{ h | help    | false | Print help message }"
};



int main( int argc, char** argv )
{


    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);

    if (cmd.get<bool>("help"))
    {
        cout << "Background Subtraction Program." << endl;
        cout << "------------------------------------------------------------" << endl;
        cout << "Process input video comparing with its ground truth.        " << endl;
        cout << "OpenCV Version : "  << CV_VERSION << endl;
        cout << "Example:                                                    " << endl;
        cout << "./bgs -i dir_jpeg/ -g ground_truth/ -c config/init.txt -s   " << endl << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Read input parameters
    const string inputVideoName  = cmd.get<string>("input");
    const string outputVideoName = cmd.get<string>("output");
    const string groundTruthName = cmd.get<string>("gtruth");
    const string bgModelName     = cmd.get<string>("read");
    const string saveName        = cmd.get<string>("write");
    const string displayPoint    = cmd.get<string>("point");
    const string rangeFrame      = cmd.get<string>("range");
    const int shiftFrame         = cmd.get<int>("frame");
    const bool displayImages     = cmd.get<bool>("show");
    const bool applyFilter       = cmd.get<bool>("filter");
    const bool debugPoint        = cmd.get<bool>("debug");
    const bool saveMask          = cmd.get<bool>("mask");
    bool verbose                 = cmd.get<bool>("verbose");

    //declaration of local variables.
    map<unsigned int, string> gt_files;
    map<unsigned int, string> im_files;
    map<unsigned int, string>::iterator it;
    map<unsigned int, string>::iterator it_im;
    Mat gt_image;
    stringstream msg,ptmsg;
    ofstream outfile, ptfile, rocfile;
    BackgroundSubtractorMOG3 bg_model;
    Mat img, fgmask, fgimg, ftimg,fgimg_final;
    bool update_bg_model = true;
    Mat frame;
    bool compare = false;
    bool show_point = false;
    Performance perf;
    int cntTemporalWindow = 0;
    int gt_cnt = 0;
    int gt_size = -1;
    int im_size = -1;
    mdgkt *filter;
    bool processing_video = false;
    unsigned int InitFGMaskFrame = 216;
    unsigned int EndFGMaskFrame  = 682;

    

    
    if (inputVideoName.empty()) {
        cout << "Insert video name" << endl;
        cmd.printParams();
        return -1;
    }
    
    //Input name could be either a video or jpeg files directory 
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

    //Load initialization parameters
    string config_filename = "config/sagmm.xml";
    
    if (!FileExists(config_filename.c_str())) {
        cout << "Configuration file " << config_filename << " not found! " << endl;
        return -1;
    }

    // Load initialization parameters
    bg_model.loadInitParametersFromXMLFile();

    //Print out initialization parameters.
    if (verbose)
        cout << bg_model.initParametersToString() << endl;

    // Read init/end ground truth values
    FileStorage fs(config_filename, FileStorage::READ);
    InitFGMaskFrame = (int)fs["InitFGMaskFrame"];
    EndFGMaskFrame  = (int)fs["EndFGMaskFrame"];
    fs.release();

    //Create foreground mask directory
    string foreground_path = "sagmm_mask";
    if (saveMask) {
        
        // Create directoty if not exists and create a numbered internal directory.
        // fgmask/0, fgmask/1, ...
        create_foreground_directory(foreground_path);
        string parameter_file = foreground_path + "/parameters.txt";
        outfile.open(parameter_file.c_str());
        outfile << bg_model.initParametersAsOneLineString() << endl;
        outfile.close();

    }
    
    //Check ground-truth
    if (!groundTruthName.empty()) {
        compare=true;
        list_files(groundTruthName,gt_files);
        outfile.open("output.txt");
        outfile << bg_model.initParametersAsOneLineString() << endl;

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

    int width;
    int height;
    int delay = 25;
    VideoCapture video;
    
    if (processing_video) {
        //create video object.
        video.open(inputVideoName);
        
        // Check video has been opened sucessfully
        if (!video.isOpened())
            return 0;
        
        double rate   = video.get(CV_CAP_PROP_FPS);
        width         = video.get(CV_CAP_PROP_FRAME_WIDTH);
        height        = video.get(CV_CAP_PROP_FRAME_HEIGHT);
        delay         = 1000/rate;
    }
    else {
        //create video object.
        Mat imfile = imread(im_files[1]);
        
        // Check file has been opened sucessfully
        if (imfile.data == NULL )
            return 0;
        
        width  = imfile.cols;
        height = imfile.rows;
    }

    //Get specific point to be displayed in the image.
    //Check input point to display value
    int nl=0,nc=0;
    Point pt(0,0);
    if (!displayPoint.empty()) {
        pt = stringToPoint(displayPoint);
        nc = pt.x > width  ? width  : pt.x;
        nl = pt.y > height ? height : pt.y;
        show_point = true;

        //define outfile to save BGR pixel values
        stringstream name(""); 
        name << "pt_" << pt.x << "_" << pt.y << ".txt";
        ptfile.open(name.str().c_str());
    }
    
    
    if (!rangeFrame.empty()) {
        Point p1;
        p1 = stringToPoint(rangeFrame);
        InitFGMaskFrame = p1.x;
        EndFGMaskFrame  = p1.y;
    }
    
    
    //check if debug option is enabled
    //this will disable verbose mode
    if (debugPoint) {
        cout << "Debug" << endl; 
        if ( pt.x == 0 && pt.y == 0 )
            pt = Point(width/2,height/2);
        
        //stop printing out messages
        verbose = false;
            
        bg_model.setPointToDebug(pt);
        
    }
    
    // Create display windows 
    if (displayImages) { 
        namedWindow("image", CV_WINDOW_NORMAL);
        namedWindow("foreground mask", CV_WINDOW_NORMAL);
        //namedWindow("foreground image", CV_WINDOW_NORMAL);
        moveWindow("image"           ,50,50);
        moveWindow("foreground mask" ,400,50);
        //moveWindow("foreground image",20,300);

    } 
    
    //spatio-temporal pre-processing filter for smoothing frames.
    if (applyFilter) {
        
        filter = mdgkt::Instance();
        cntTemporalWindow = filter->getTemporalWindow();

        for (int i=0; i<cntTemporalWindow; i++) {

            frame = Scalar::all(0);
            if (processing_video)
                video >> frame;
            else
                frame = imread(im_files[i+1]);

            // Initialize in zero three channels of img kernel.
            if (i==0)
                filter->initializeFirstImage(frame);
            
            //Apply filter and puts result in img.
            //Note this filter also keeps internal copy of filter result.
            filter->SpatioTemporalPreprocessing(frame, img);
            

        }
    }
    else {
        if (processing_video) {
            VideoCapture tmp_video(inputVideoName);
            tmp_video >> img;
            tmp_video.release();
        }
        else
            img = imread(im_files[1], CV_LOAD_IMAGE_COLOR);
    } 
    
    bg_model.initializeModel(img);
    //bg_model.loadModel();
    Mat bgimg;
    bg_model.getBackground(bgimg);

    //Shift backward or forward ground truth sequence counter.
    //for compensating pre-processed frames in the filter.
    unsigned int cnt    = 0  + shiftFrame + cntTemporalWindow; 


    // main loop 
    for(;;)
    {
        //cout << cnt << " " ;

        //clean up all Mat structures.
        //This is done because, performance was improved.
        img    = Scalar::all(0);
        frame  = Scalar::all(0);
        fgmask = Scalar::all(0);
        
        //before processing image get background
        
        
        //Checks if filter option was enabled.
        if (applyFilter) {

            if (processing_video)
                video >> frame;
            else
                frame = imread(im_files[cnt+1]);

            if (frame.empty()) 
                break;

            //Applies spatial and temporal filter
            //note this filter return a Mat CV_32FC3 type.
            filter->SpatioTemporalPreprocessing(frame, img);
        }
        else {
            if (processing_video)
                video >> img;
            else
                img = imread(im_files[cnt+1]);

            if (img.empty()) 
                break;
        } 
            
        if( fgimg.empty() )
            fgimg.create(img.size(), img.type());
        
        //Global illumination changing factor 'g' between reference image ir and current image ic.
        double globalIlluminationFactor = icdm::Instance()->getIlluminationFactor(img,bgimg);

        //Calling background subtraction algorithm.
        bg_model(img, fgmask, update_bg_model ? -1 : 0, globalIlluminationFactor);
        
        bg_model.getBackgroundImage(bgimg);

        fgimg = Scalar::all(0);
        
        img.copyTo(fgimg, fgmask);
        
        //save mask to local directory
        if (saveMask  && cnt >= InitFGMaskFrame && cnt <= EndFGMaskFrame) {
            stringstream str;
            str << foreground_path << "/" <<  cnt << ".jpg";
            
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            compression_params.push_back(100);
            imwrite(str.str(), fgmask, compression_params);
        }
       


        //looking for ground truth file
        if ( cnt >=0 && compare && (it = gt_files.find(cnt)) != gt_files.end() ) {

            // open ground truth frame.
            gt_image = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);

            if( gt_image.data ) {

                //Compare both images
                perf.pixelLevelCompare(gt_image, fgmask);

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

            // counter of number of frame processed
            gt_cnt++;
        }

       
        // Save pixel information in a local file
        if (show_point) {
            Mat ptimg;
            img.convertTo(ptimg,CV_8UC3);
            ptmsg.str("");
            ptmsg  << (int)ptimg.at<Vec3b>(nl,nc)[0] << " " 
                   << (int)ptimg.at<Vec3b>(nl,nc)[1] << " " 
                   << (int)ptimg.at<Vec3b>(nl,nc)[2] ;
            ptfile << ptmsg.str() << endl;
        }


        //Display sequences
        if (displayImages) {
            img.convertTo(ftimg, CV_8UC3);
            if (show_point){
                circle(ftimg,pt,8,Scalar(0,0,254),-1,8);
                circle(fgmask,pt,8,Scalar(155,155,155),-1,8);
            }
            
            imshow("image", ftimg);
            imshow("foreground mask", fgmask);
            //imshow("foreground image", fgimg);


            // This is just to write down two different images, showing red point on them.
            // This line could be commented out.
            //if ((cnt >= 605) &&  (cnt <= 611)) {
            //if (cnt == 570) {
            //    stringstream str;
            //    str <<  cnt << ".jpg";
            //    imwrite(str.str(), ftimg);
            //    stringstream fgstr;
            //    fgstr <<  cnt << "_fg.jpg";
            //    imwrite(fgstr.str(), fgmask);

            //    Mat small_img;
            //    img.convertTo(small_img,CV_8UC3);
            //    Mat imageROI = small_img(cv::Rect((pt.x-3),(pt.y-3),6,6));
            //    cout << setprecision(0) << fixed << imageROI << endl;
            //}

            char key=0;
            key = (char)waitKey(delay);
            if( key == 27 ) 
                break;        
        
        }

 
        cnt++;
        

        //in case of not display option enabled stop execution 
        //after last ground-truth file was processed
        if (!displayImages && cnt > EndFGMaskFrame)
            break;

        //save model
        //if (cnt == 500)
        //    bg_model.saveModel();
        
    }


    if (!groundTruthName.empty() && compare) {
        
        perf.calculateFinalPerformanceOfMetrics();
        
        cout    << perf.metricsStatisticsAsString() << endl;
        outfile << "# TPR FPR SPE MCC  TPR TNR SPE MCC" << endl;
        outfile << "# " << perf.metricsStatisticsAsString() << endl;
        outfile.close();
        rocfile.open("roc.txt", std::fstream::out | std::fstream::app);
        rocfile << bg_model.getRange() << " " 
                << bg_model.getAlpha() << " "  
                << perf.rocAsString() << endl;
        rocfile.close();
    }

    if (ptfile.is_open())
        ptfile.close();

    return 0;
}

