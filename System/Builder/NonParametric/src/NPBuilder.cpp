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
#include "NPBuilder.h"


const int           NPBuilder::DefaultFramesToLearn            = 10;
const int           NPBuilder::DefaultSequenceLength           = 50;
const int           NPBuilder::DefaultTimeWindowSize           = 100;
const unsigned char NPBuilder::DefaultUpdateFlag               = 1;
const unsigned char NPBuilder::DefaultSDEstimationFlag         = 1;
const unsigned char NPBuilder::DefaultUseColorRatiosFlag       = 1;
const double        NPBuilder::DefaultThreshold                = 10.0e-8;
const double        NPBuilder::DefaultAlpha                    = 0.3;
const unsigned int  NPBuilder::DefaultInitFGMaskFrame          = 216;
const unsigned int  NPBuilder::DefaultEndFGMaskFrame           = 682;
const unsigned char NPBuilder::DefaultApplyMorphologicalFilter = 1;


NPBuilder::NPBuilder()
{
    loadDefaultParameters();
    cols = 0;
    rows = 0;
    nchannels = 0;
    has_been_initialized = false;
    frame_counter = 0;

}

NPBuilder::NPBuilder(int _col, int _row, int _nchannels)
{
    loadDefaultParameters();
    cols = _col;
    rows = _row;
    nchannels = _nchannels;
    has_been_initialized = false;
    frame_counter = 0;

}

NPBuilder::NPBuilder(Size _size, int _type)
{
    loadDefaultParameters();
    frameSize = _size;
    frameType = _type;
    rows = frameSize.height;
    cols = frameSize.width;
    nchannels = CV_MAT_CN(frameType);
    has_been_initialized = false;
    frame_counter = 0;
}


NPBuilder::~NPBuilder()
{
    if (model != NULL) {
        delete model;
    }
    model = 0;

    if (has_been_initialized) {
        delete [] DisplayBuffers[0];   // First delete pointer content
        delete [] DisplayBuffers[1];
        delete [] DisplayBuffers[2];
        delete [] DisplayBuffers[3];
        delete [] DisplayBuffers[4];
        delete [] DisplayBuffers;

        delete FilterFGImage;
    }
}

void NPBuilder::Initialization()
{

    if (rows != 0 && cols != 0 && nchannels != 0) {
        model = new NPBGSubtractor;
        model->Intialize(rows, cols, nchannels, SequenceLength, TimeWindowSize, SDEstimationFlag, UseColorRatiosFlag);
        model->SetThresholds(Threshold,Alpha);
        model->SetUpdateFlag(UpdateFlag);

        has_been_initialized = true;

        // Prepare buffers
        DisplayBuffers = new unsigned char*[5];
        DisplayBuffers[0] = new unsigned char[rows*cols];
        DisplayBuffers[1] = new unsigned char[rows*cols];
        DisplayBuffers[2] = new unsigned char[rows*cols];
        DisplayBuffers[3] = new unsigned char[rows*cols];
        DisplayBuffers[4] = new unsigned char[rows*cols];

        FilterFGImage = new unsigned char[cols*rows];
    }
    
}

string NPBuilder::PrintParameters()
{
    std::stringstream str;
    str
    << "# "
    << "Alpha="          << Alpha          << " "
    << "Threshold="      << Threshold      << " "
    << "FramesToLearn="  << FramesToLearn  << " " 
    << "SequenceLength=" << SequenceLength << " "
    << "TimeWindowSize=" << TimeWindowSize;
    return str.str();

}

void NPBuilder::LoadConfigParameters()
{
    string filename = "config/np.xml";

    if ( exists(filename) && is_regular_file(filename) ) {
        
        FileStorage fs(filename, FileStorage::READ);
       
        FramesToLearn            = (int)fs["FramesToLearn"];
        SequenceLength           = (int)fs["SequenceLength"] ;
        TimeWindowSize           = (int)fs["TimeWindowSize"] ;
        UpdateFlag               = (int)fs["UpdateFlag"]; 
        SDEstimationFlag         = (int)fs["SDEstimationFlag"] ;
        UseColorRatiosFlag       = (int)fs["UseColorRatiosFlag"] ;
        Threshold                = (double)fs["Threshold"]; 
        Alpha                    = (double)fs["Alpha"];
        InitFGMaskFrame          = (int)fs["InitFGMaskFrame"]; 
        EndFGMaskFrame           = (int)fs["EndFGMaskFrame"]; 
        ApplyMorphologicalFilter = (int)fs["ApplyMorphologicalFilter"]; 

        fs.release();
    }
    else {
        path p("config");
        if (!exists(p))
            create_directory(p);
        
        FileStorage fs(filename, FileStorage::WRITE);

        fs << "FramesToLearn"            << (int)FramesToLearn;
        fs << "SequenceLength"           << (int)SequenceLength;
        fs << "TimeWindowSize"           << (int)TimeWindowSize;
        fs << "UpdateFlag"               << (int)UpdateFlag; 
        fs << "SDEstimationFlag"         << (int)SDEstimationFlag;
        fs << "UseColorRatiosFlag"       << (int)UseColorRatiosFlag;
        fs << "Threshold"                << Threshold; 
        fs << "Alpha"                    << Alpha;
        fs << "InitFGMaskFrame"          << (int)InitFGMaskFrame; 
        fs << "EndFGMaskFrame"           << (int)EndFGMaskFrame; 
        fs << "ApplyMorphologicalFilter" << (int)ApplyMorphologicalFilter; 

        fs.release();

    }

}

void NPBuilder::GetBackground(OutputArray image) 
{

}

void NPBuilder::GetForeground(OutputArray mask) 
{

}

void NPBuilder::Update(InputArray frame, OutputArray mask) 
{
    Mat Image = frame.getMat();

        
    // Check model has not been initialized.
    if ( !has_been_initialized ) {
        Size frameSize = Image.size();
        int  frameType = Image.type();
        rows = frameSize.height;
        cols = frameSize.width;
        nchannels = CV_MAT_CN(frameType);
        Initialization();
    }

    // Add initial frames to learn
    if (frame_counter < FramesToLearn) {
        model->AddFrame(Image.data);
        frame_counter += 1;
        
        Mat Zero(Image.size(),CV_8U,Scalar::all(0));
        Zero.copyTo(mask);
    }
    else {

        // Make estimation
        if (frame_counter == FramesToLearn) {
            model->Estimation();
        }

        Mat Foreground(Image.size(),CV_8U,Scalar::all(0));

        //subtract the background from each new frame
        ((NPBGSubtractor *)model)->NBBGSubtraction(Image.data, Foreground.data, FilterFGImage, DisplayBuffers);

        //here you pass a mask where pixels with true value will be masked out of the update.
        ((NPBGSubtractor *)model)->Update(Foreground.data);

        // Applying morphological filter
        // Erode the image
        Mat Eroded; // the destination image
        if (ApplyMorphologicalFilter) {
            Mat Element(2,2,CV_8U,Scalar(1));
            //erode(Mask,Eroded,Mat());
            erode(Foreground,Eroded,Element);
            Eroded.copyTo(mask);
        }
        else {
            Foreground.copyTo(mask);
        }

        frame_counter += 1;
    }

}


void NPBuilder::loadDefaultParameters()
{
    FramesToLearn            = DefaultFramesToLearn;
    SequenceLength           = DefaultSequenceLength ;
    TimeWindowSize           = DefaultTimeWindowSize ;
    UpdateFlag               = DefaultUpdateFlag; 
    SDEstimationFlag         = DefaultSDEstimationFlag ;
    UseColorRatiosFlag       = DefaultUseColorRatiosFlag ;
    Threshold                = DefaultThreshold; 
    Alpha                    = DefaultAlpha;
    InitFGMaskFrame          = DefaultInitFGMaskFrame; 
    EndFGMaskFrame           = DefaultEndFGMaskFrame; 
    ApplyMorphologicalFilter = DefaultApplyMorphologicalFilter; 
}

string NPBuilder::ElapsedTimeAsString()
{
    std::stringstream _elapsed ;
    _elapsed << duration ;
    return _elapsed.str() ;
}



