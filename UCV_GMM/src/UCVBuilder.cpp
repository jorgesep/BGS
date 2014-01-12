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

#include "UCVBuilder.h"
#include "BGSTimer.h"


const double       UCVBuilder::DefaultThreshold                  = 16;
const double       UCVBuilder::DefaultLearningRate               = 0.001;
const unsigned int UCVBuilder::DefaultInitFGMaskFrame            = 216;
const unsigned int UCVBuilder::DefaultEndFGMaskFrame             = 682;
const unsigned int UCVBuilder::DefaultApplyMorphologicalFilter   = 0;
const unsigned int UCVBuilder::DefaultTypeFunction               = 1;
const unsigned int UCVBuilder::DefaultNumberGaussians            = 3;
const unsigned int UCVBuilder::DefaultColumns                    = 720;
const unsigned int UCVBuilder::DefaultRows                       = 576;
const unsigned int UCVBuilder::DefaultNumberChannels             = 3;



UCVBuilder::~UCVBuilder()
{
    //Saved elapsed time in a file.
    BGSTimer::Instance()->registerStopTime();
    BGSTimer::Instance()->getSequenceElapsedTime();
    BGSTimer::deleteInstance();

    // Later I will convert this part in a Factory Pattern
    if (TypeFunction == 1) {
        if (linear != NULL) {
            delete linear;
            linear = 0;
        }
    }
    else if (TypeFunction == 2) {
        if (staircase != NULL) {
            delete staircase;
            staircase = 0;
        }
    }
    else {
        if (gmm != NULL) {
            delete gmm;
            gmm = 0;
        }
    }

    if (has_been_initialized)
        delete [] current_frame_ptr ;
}

UCVBuilder::UCVBuilder()
{
    loadDefaultParameters ();
}


UCVBuilder::UCVBuilder(int _col, int _row, int _nchannels)
{
    loadDefaultParameters();

    cols = _col;
    rows = _row;
    len  = cols*rows;
    nchannels = _nchannels;
    has_been_initialized = false;
    frame_counter = 0;

}

UCVBuilder::UCVBuilder(Size _size, int _type)
{
    loadDefaultParameters();

    frameSize = _size;
    frameType = _type;
    rows = frameSize.height;
    cols = frameSize.width;
    len  = cols*rows;
    nchannels = CV_MAT_CN(frameType);
    has_been_initialized = false;
    frame_counter = 0;
}

void UCVBuilder::Initialization()
{
    current_frame_ptr = new unsigned char [rows*cols] ;
    current_frame.image = current_frame_ptr ;
    current_frame.type  = UCV_GRAY8U ;
    current_frame.width = cols ;
    current_frame.height= rows ;

    // Later I will convert this part in a Factory Pattern
    if (TypeFunction == 1) {
        linear = new gmm_line(&current_frame, \
                              Threshold, \
                              LearningRate, \
                              (uint8_t)NumberGaussians);
    }
    else if (TypeFunction == 2) {
        staircase = new gmm_sample(&current_frame, \
                Threshold, \
                LearningRate, \
                (uint8_t)NumberGaussians);
    }
    else {
        gmm = new gmm_double_ucv(&current_frame, \
                Threshold, \
                LearningRate, \
                (uint8_t)NumberGaussians);
    }

    // Just for now, I'm going a start timer here (workaround)
    string description = Name() + PrintParameters();
    BGSTimer::Instance()->setSequenceName( "Framework_UCV", description) ;
    BGSTimer::Instance()->registerStartTime();
    
}

string UCVBuilder::PrintParameters()
{
    std::stringstream str;
    str 
    << "# "
    << "LearningRate=" << LearningRate  << " "
    << "Threshold="    << Threshold     << " "
    << "Function="     << TypeFunction;
    return str.str();
    
}


void UCVBuilder::LoadConfigParameters()
{
    string filename = "config/ucv.xml";

    if ( exists(filename) && is_regular_file(filename) ) {
        
        FileStorage fs(filename, FileStorage::READ);

        NumberGaussians          =   (int)fs["NumberGaussians"];
        Threshold                =   (int)fs["Threshold"];                  
        LearningRate             =   (double)fs["LearningRate"];                    
        InitFGMaskFrame          =   (int)fs["InitFGMaskFrame"];         
        EndFGMaskFrame           =   (int)fs["EndFGMaskFrame"];           
        ApplyMorphologicalFilter =   (int)fs["ApplyMorphologicalFilter"]; 
        TypeFunction             =   (float)fs["TypeFunction"];             
       
        fs.release();
    }
    else {
        path p("config");
        if (!exists(p))
            create_directory(p);
        
        FileStorage fs(filename, FileStorage::WRITE);
        fs << "NumberGaussians"          << (int)NumberGaussians;
        fs << "Threshold"                << (float)Threshold;
        fs << "LearningRate"             << (double)LearningRate;
        fs << "InitFGMaskFrame"          << (int)InitFGMaskFrame;
        fs << "EndFGMaskFrame"           << (int)EndFGMaskFrame;
        fs << "ApplyMorphologicalFilter" << (int)ApplyMorphologicalFilter;
        fs << "Typefunction"             << (int)TypeFunction;
        fs.release();

    }
}

void UCVBuilder::GetBackground(OutputArray image) 
{

}

void UCVBuilder::GetForeground(OutputArray mask) 
{

}

void UCVBuilder::Update(InputArray frame, OutputArray foreground) 
{
    
    Mat Image = frame.getMat();
    Mat Image_gray(rows,cols,CV_8U);

    foreground.create(rows,cols,CV_8U);
    Mat Mask = foreground.getMat();
    Mask     = Scalar::all(0);

    if (Image.empty()) return;

    // Check and convert reference image to gray
    Image_gray = Image;
    if (Image.channels() > 1)
        cvtColor( Image, Image_gray, CV_BGR2GRAY );

    memcpy(UCV_IMAGE_DATA(current_frame), (unsigned char*)Image_gray.data, len);
    if (TypeFunction == 1)
        linear->process   (&current_frame, (uint8_t*)Mask.data);
    else if (TypeFunction == 2)
        staircase->process(&current_frame, (uint8_t*)Mask.data);
    else
        gmm->process      (&current_frame, (uint8_t*)Mask.data);


    // Applying morphological filter (Erode) in case option was enabled.
    Mat filtered_mask; // the destination image
    if (ApplyMorphologicalFilter) {
        Mat Element(2,2,CV_8U,cv::Scalar(1));
        erode(Mask,filtered_mask,Element);
        Mask  = Scalar::all(0);
        filtered_mask.copyTo(Mask);
    }

}

string UCVBuilder::ElapsedTimeAsString()
{
    std::stringstream _elapsed ;
    _elapsed << duration;
    return _elapsed.str();

}

void UCVBuilder::loadDefaultParameters()
{
    Threshold                = DefaultThreshold;             
    LearningRate             = DefaultLearningRate;                    
    InitFGMaskFrame          = DefaultInitFGMaskFrame;          
    EndFGMaskFrame           = DefaultEndFGMaskFrame  ;         
    ApplyMorphologicalFilter = DefaultApplyMorphologicalFilter ;
    TypeFunction             = DefaultTypeFunction  ;                

    cols                     = DefaultColumns;
    rows                     = DefaultRows;
    nchannels                = DefaultNumberChannels;
    len                      = cols*rows;
}

