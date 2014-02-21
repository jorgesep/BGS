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

//
//#include "Performance.h"
#include "utils.h"
#include "BGSTimer.h"

#include "ucv_types.h"
#include "ucv_gmm_data.h"
#include "ucv_gmm_d.h"
#include "ucv_gmm.h"

#include "gmm_line.h"
#include "gmm_sample.h"
#include "gmm_double_ucv.h"


#include "utils.h"
#include "FrameReaderFactory.h"



using namespace cv;
using namespace std;
using namespace boost::filesystem;
//using namespace seq;
using namespace bgs;



void create_g_img(ucv_image_t *a, uint8_t *a_b, uint16_t w, uint16_t h)
{
    a->image = a_b;
    a->type = UCV_GRAY8U;
    a->width = w;
    a->height = h;
}

void create_ucv_img(ucv_image_t *a, uint8_t *a_b, uint16_t w, uint16_t h)
{
        a->image = a_b;
        a->type = UCV_GRAY8U;
        a->width = w;
        a->height = h;
}





const char* keys =
{
    "{ i | input   |       | Input video }"
    "{ m | mask    | true  | Save foreground masks}"
    "{ f | function| 1     | Type of method: 1:linear 2:staircase 3:gmm normal}"
    "{ s | show    | false | Show images of video sequence }"
    "{ h | help    | false | Print help message }"
};




int main( int argc, char** argv )
{
    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);

    if (cmd.get<bool>("help"))
    {
        cout << "uCV Model for background subtraction Program." << endl;
        cout << "------------------------------------------------------------" << endl;
        cout << "Generates foreground masks.                                 " << endl;
        cout << "OpenCV Version : "  << CV_VERSION << endl;
        cout << "Example:                                                    " << endl;
        cout << "./test_ucv -i dir_jpeg/  -s                  " << endl << endl;
        cmd.printParams();
        cout << "------------------------------------------------------------" << endl <<endl;
        return 0;
    }

    // Reading console input parameters
    const string Name             = cmd.get<string>("input");
    const bool displayImages      = cmd.get<bool>("show");
    const bool saveForegroundMask = cmd.get<bool>("mask");
    const int  typeFunction       = cmd.get<int>("function");




    // Instance objet to verify input name is either a video or image directory.
    seq::FrameReader *input_frame;
    try {
        input_frame = seq::FrameReaderFactory::create_frame_reader(Name);
    } catch (...) {
        cout << "Invalid file name "<< endl;
        return 0;
    }

    stringstream name;
    stringstream _config ;
    stringstream _foreground;
    if (typeFunction == 1)
        name << "ucv_linear"; 
    else if (typeFunction == 2)
        name << "ucv_staircase"; 
    else if (typeFunction == 3)
        name << "ucv_gmm"; 
    else
        name << "ucv";

    // Load xml file with parameters of algorithm needed to initialize the program.
    _config << "config/" << name.str() << ".xml" ;
    string config_filename = _config.str() ; 
    path config_path(config_filename);
    if (!is_regular_file(config_path)) {
        cout << "Configuration file " << config_filename << " not found! " << endl;
        return -1;
    }

    // Reading parameters of algorithm from xml file.
    FileStorage fs(config_filename, FileStorage::READ);
    double LearningRate              = (double)fs["LearningRate"];
    double Threshold                 = (double)fs["Threshold"];
    int NumberGaussians              = (int)fs["NumberGaussians"];
    int InitFGMaskFrame              = (int)fs["InitFGMaskFrame"];
    int EndFGMaskFrame               = (int)fs["EndFGMaskFrame"];
    int ApplyMorphologicalFilter     = (int)fs["ApplyMorphologicalFilter"];
    fs.release();

    // Create foreground mask directory
    _foreground << name.str() << "_mask" ;
    string foreground_path = _foreground.str() ; 
    if (saveForegroundMask) {

        // Create directoty if not exists and create a numbered internal directory.
        // np_mask/0, np_mask/1, ...
        create_foreground_directory(foreground_path);

        ofstream outfile;
        stringstream param;
        param << foreground_path << "/parameters.txt" ;
        outfile.open(param.str().c_str());
        outfile << "# LearningRate=" << LearningRate << " Threshold=" << Threshold ;
        outfile.close();

    }


    // Creation of display windows
    if (displayImages) {
        namedWindow("Image", CV_WINDOW_NORMAL);
        namedWindow("Foreground", CV_WINDOW_NORMAL);
        moveWindow("Image", 20, 20);
        moveWindow("Foreground" , 20, 300);
    }


    // Image properties.
    int cols      = input_frame->getNumberCols();
    int rows      = input_frame->getNumberRows();
    //int nchannels = input_frame->getNChannels();
    int delay     = input_frame->getFrameDelay();
    int len       = rows * cols;




    /* Start configuration of the algorithm */
    ucv_image_t curr;
    uint8_t* curr_b = new uint8_t[len];

    // Read first frame to configure algorithm
    //Mat Frame(rows,cols,CV_8U);
    Mat Frame;
    Mat Frame_gray(rows,cols,CV_8U);
    Mat Mask(rows,cols,CV_8U);

    input_frame->getFrame(Frame);
    Mat gray_image = Frame;
    if (Frame.channels() > 1)
        cvtColor( Frame, gray_image, CV_BGR2GRAY );


    unsigned char *curr_c = new unsigned char[len];
    memcpy(curr_c, (uint8_t*)gray_image.data, len);
    create_ucv_img(&curr, curr_b, cols, rows);
    gmm_line   *g_l; 
    gmm_sample *g_s;
    gmm_double_ucv *g_d;

    if      (typeFunction == 1)
        g_l = new gmm_line      (&curr, Threshold, LearningRate, (uint8_t)NumberGaussians);
    else if      (typeFunction == 2)
        g_s = new gmm_sample    (&curr, Threshold, LearningRate, (uint8_t)NumberGaussians);
    else if (typeFunction == 3)
        g_d = new gmm_double_ucv(&curr, Threshold, LearningRate, (uint8_t)NumberGaussians);
    else {
        cout << "Invalid function " << endl;
        return 1;
    }
     
    // Initialize counter of number of frames read.
    unsigned int cnt = input_frame->getFrameCounter();

    // Start timer
    stringstream description;
    string ucv_name ; 
    if (typeFunction == 1)
        ucv_name = "UCV_LINEAR";
    else if (typeFunction == 2)
        ucv_name = "UCV_STAIRCASE";
    else
        ucv_name = "UCV_NORMAL";

    description << ucv_name << " # LearningRate=" << LearningRate << " Threshold=" << Threshold ;
    BGSTimer::Instance()->setSequenceName(Name, description.str()) ;
    BGSTimer::Instance()->registerStartTime();

    // main loop
    for(;;)
    {

        // Read input frame
        Mask  = Scalar::all(0);
        Frame = Scalar::all(0);
        input_frame->getFrame(Frame);
        if (Frame.empty()) break;

        // Check and convert reference image to gray
        Frame_gray = Frame;
        if (Frame.channels() > 1)
            cvtColor( Frame, Frame_gray, CV_BGR2GRAY );

        cnt = input_frame->getFrameCounter();

        // Process the image
        memcpy(UCV_IMAGE_DATA(curr), (unsigned char*)Frame_gray.data, len);
        if (typeFunction == 1)
            g_l->process(&curr, (uint8_t*)Mask.data);
        else if (typeFunction == 2)
            g_s->process(&curr, (uint8_t*)Mask.data);
        else
            g_d->process(&curr, (uint8_t*)Mask.data);
 

        // Applying morphological filter (Erode) in case option was enabled.
        Mat filtered_mask; // the destination image
        if (ApplyMorphologicalFilter) {
            Mat Element(2,2,CV_8U,cv::Scalar(1));
            erode(Mask,filtered_mask,Element);
        }
        else {
            Mask.copyTo(filtered_mask);
        }

        // Save foreground mask image for posterior analysis.
        if (saveForegroundMask &&  cnt >= (unsigned int)InitFGMaskFrame &&  cnt <= (unsigned int)EndFGMaskFrame) {
            stringstream str;
            //str << foreground_path << "/" <<  cnt << ".jpg";
            str << foreground_path << "/" <<  cnt << ".png";

            //vector<int> compression_params;
            //compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            //compression_params.push_back(100);
            ////imwrite(str.str(), Mask, compression_params);
            //imwrite(str.str(), filtered_mask, compression_params);


            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(9);

            try {
                imwrite(str.str(), filtered_mask, compression_params);
            }
            catch (runtime_error& ex) {
                cout << "Exception converting image to PNG format: " << ex.what() << endl;
            }
            catch (...) {
                cout << "Unknown Exception converting image to PNG format: " << endl;
            }





        }

        // Display foreground mask and image
        if (displayImages) {
            imshow("Image"     , Frame_gray);
            imshow("Foreground", filtered_mask);

            char key=0;
            key = (char)waitKey(delay);
            if( key == 27 )
                break;
        }

        // option display not enabled stop loop after check last gt image.
        if (!displayImages && cnt > (unsigned int)EndFGMaskFrame)
            break;



    }


    // Return elapsed time and sve it in file.
    BGSTimer::Instance()->registerStopTime();
    cout << BGSTimer::Instance()->getSequenceElapsedTime() << endl;
    BGSTimer::deleteInstance();

    delete input_frame;

    return 0;
}

