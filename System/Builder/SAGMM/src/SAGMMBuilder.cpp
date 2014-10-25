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
#include "SAGMMBuilder.h"
//#include <iostream>
//#include <vector>
//#include <fstream>



const int           SAGMMBuilder::DefaultGaussiansNo              = 4;
const double        SAGMMBuilder::DefaultSigma                    = 36;
const double        SAGMMBuilder::DefaultSigmaMax                 = 3.0;
const double        SAGMMBuilder::DefaultSigmaMin                 = 4.0;
const double        SAGMMBuilder::DefaultCf                       = 0.01;
const double        SAGMMBuilder::DefaultRange                    = 16;
const double        SAGMMBuilder::DefaultGen                      = 9;
const double        SAGMMBuilder::DefaultCT                       = 0.05;
const double        SAGMMBuilder::DefaultTau                      = 0.5;
const double        SAGMMBuilder::DefaultAlpha                    = 0.0001;
const unsigned int  SAGMMBuilder::DefaultInitFGMaskFrame          = 216;
const unsigned int  SAGMMBuilder::DefaultEndFGMaskFrame           = 682; 
const unsigned char SAGMMBuilder::DefaultApplyMorphologicalFilter = 1;






SAGMMBuilder::~SAGMMBuilder()
{
    if (model != NULL) {
        delete model;
    }
    model = 0;
    filter->deleteInstance();
    factor->deleteInstance();
}

SAGMMBuilder::SAGMMBuilder()
{

    GaussiansNo              = DefaultGaussiansNo;
    Sigma                    = DefaultSigma;
    SigmaMax                 = DefaultSigmaMax;
    SigmaMin                 = DefaultSigmaMin;
    Cf                       = DefaultCf;
    Range                    = DefaultRange;
    Gen                      = DefaultGen;
    CT                       = DefaultCT;
    Tau                      = DefaultTau;
    Alpha                    = DefaultAlpha;
    InitFGMaskFrame          = DefaultInitFGMaskFrame;
    EndFGMaskFrame           = DefaultEndFGMaskFrame;
    ApplyMorphologicalFilter = DefaultApplyMorphologicalFilter;

    frame_counter = 0;
    has_been_initialized = false;
    update_bg_model = true;

}

void SAGMMBuilder::Initialization()
{
    model = new BackgroundSubtractorMOG3();

    //In case xml file not exist this command will create it.
    LoadConfigParameters();

    model->loadInitParametersFromXMLFile();

    // Get instance of spatio temporal filter
    filter = mdgkt::Instance();

    // Get instance of illumination factor object
    factor = icdm::Instance();
    
}


string SAGMMBuilder::PrintParameters()
{
    stringstream str;
    //# Alpha=0.01 cf=0.1 bgRation=0.9 Range=10 Gen=9 GaussiansNo=4 Sigma=36 cT=0.05 Tau=0.5
    str 
    << "# "
    << "Alpha="       << Alpha       << " "
    << "cf="          << Cf          << " "
    << "bgRation="    << 1-Cf        << " "
    << "Range="       << Range       << " "
    << "Gen="         << Gen         << " "
    << "GaussiansNo=" << GaussiansNo << " "
    << "Sigma="       << Sigma       << " "
    << "cT="          << CT          << " "
    << "Tau="         << Tau;
    return str.str();
}



void SAGMMBuilder::LoadConfigParameters()
{
    string filename = "config/sagmm.xml";

    if ( exists(filename) && is_regular_file(filename) ) {
        
        FileStorage fs(filename, FileStorage::READ);

        GaussiansNo              = (int)fs["GaussiansNo"];
        Sigma                    = (double)fs["Sigma"];
        SigmaMax                 = (double)fs["SigmaMax"];
        SigmaMin                 = (double)fs["SigmaMin"];
        Cf                       = (double)fs["Cf"];
        Range                    = (double)fs["Range"];
        Gen                      = (double)fs["Gen"];
        CT                       = (double)fs["CT"];
        Tau                      = (double)fs["Tau"];
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

        fs << "GaussiansNo"              << (int)GaussiansNo;
        fs << "Sigma"                    << (double)Sigma;
        fs << "SigmaMax"                 << (double)SigmaMax;
        fs << "SigmaMin"                 << (double)SigmaMin;
        fs << "Cf"                       << (double)Cf;
        fs << "Range"                    << (double)Range;
        fs << "Gen"                      << (double)Gen;
        fs << "CT"                       << (double)CT;
        fs << "Tau"                      << (double)Tau;
        fs << "Alpha"                    << (double)Alpha;
        fs << "InitFGMaskFrame"          << (int)InitFGMaskFrame;
        fs << "EndFGMaskFrame"           << (int)EndFGMaskFrame;
        fs << "ApplyMorphologicalFilter" << (int)ApplyMorphologicalFilter;

        fs.release();

    }

}

void SAGMMBuilder::GetBackground(OutputArray image) 
{

}

void SAGMMBuilder::GetForeground(OutputArray mask) 
{

}

void SAGMMBuilder::Update(InputArray frame, OutputArray mask) 
{
    Mat Image = frame.getMat();
    Mat Foreground(Image.size(),CV_8U,Scalar::all(0));
    Mat FilteredImage;
    
    
    // Initialize temporal-spatial filter.
    if (frame_counter < filter->getTemporalWindow()) {
        
        if (frame_counter == 0) {
            // Initialize in zero three channels of img kernel.
            filter->initializeFirstImage(Image);
            
        }

        //Apply filter and puts result in FilteredImage.
        //Note this filter also keeps internal copy of filter result.
        filter->SpatioTemporalPreprocessing(Image, FilteredImage);
        
        Foreground.copyTo(mask);

        frame_counter += 1;
        
        if ( frame_counter == filter->getTemporalWindow() ) {
            
            // Initialize model
            model->initializeModel(FilteredImage);
            model->getBackground(Background);
            
        }

        return;
    }
    
    
    
    //Applies spatial and temporal filter
    //note this filter return a Mat CV_32FC3 type.
    filter->SpatioTemporalPreprocessing(Image, FilteredImage);
    
    //Global illumination changing factor 'g' between reference image ir and current image ic.
    double globalIlluminationFactor = factor->getIlluminationFactor(FilteredImage,Background);
    
    //Calling background subtraction algorithm.
    model->operator()(FilteredImage, Foreground, update_bg_model ? -1 : 0, globalIlluminationFactor);

    // background to calculate illumination next iteration.
    model->getBackground(Background);


    // Applying morphological filter, Erode the image
    Mat Eroded; 
    if (ApplyMorphologicalFilter) {
        Mat Element(2,2,CV_8U,Scalar(1));
        //erode(Mask,Eroded,Mat());
        erode(Foreground,Eroded,Element);
        Eroded.copyTo(mask);
    }
    else {
        // return mask
        Foreground.copyTo(mask);
    }

    
    
    Foreground.copyTo(mask);
    
    frame_counter += 1;
    

}


string SAGMMBuilder::ElapsedTimeAsString()
{
    std::stringstream _elapsed ;
    _elapsed << duration ;
    return _elapsed.str();

}


