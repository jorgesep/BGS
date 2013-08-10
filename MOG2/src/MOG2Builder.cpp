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
#include "MOG2Builder.h"


const int           MOG2Builder::DefaultHistory                    = 500;
const float         MOG2Builder::DefaultVarThreshold               = 16;
const bool          MOG2Builder::DefaultShadowDetection            = true;
const double        MOG2Builder::DefaultAlpha                      = 0.03;
const unsigned int  MOG2Builder::DefaultInitFGMaskFrame            = 216;
const unsigned int  MOG2Builder::DefaultEndFGMaskFrame             = 682;
const unsigned char MOG2Builder::DefaultApplyMorphologicalFilter   = 1;




MOG2Builder::~MOG2Builder()
{
    if (model != NULL) {
        delete model;
    }
    model = 0;
}

MOG2Builder::MOG2Builder()
{


    History                  = DefaultHistory  ;                
    VarThreshold             = DefaultVarThreshold;             
    ShadowDetection          = DefaultShadowDetection;          
    Alpha                    = DefaultAlpha;                    
    InitFGMaskFrame          = DefaultInitFGMaskFrame;          
    EndFGMaskFrame           = DefaultEndFGMaskFrame  ;         
    ApplyMorphologicalFilter = DefaultApplyMorphologicalFilter ;
}

void MOG2Builder::Initialization()
{
    model = new BackgroundSubtractorMOG2(History, VarThreshold, ShadowDetection);

    
    
}

string MOG2Builder::PrintParameters()
{
    std::stringstream str;
    //# Alpha=0.1 Threshold=0.0001 FramesToLearn=10 SequenceLength=50 TimeWindowSize=100
    str 
    << "# "
    << "Alpha="        << Alpha        << " "
    << "VarThreshold=" << VarThreshold << " "
    << "History="      << History;
    return str.str();
    
}


void MOG2Builder::LoadConfigParameters()
{
    string filename = "config/mog2.xml";

    if ( exists(filename) && is_regular_file(filename) ) {
        
        FileStorage fs(filename, FileStorage::READ);

        History                  =   (int)fs["History"];                  
        VarThreshold             =   (float)fs["VarThreshold"];             
        Alpha                    =   (double)fs["Alpha"];                    
        InitFGMaskFrame          =   (int)fs["InitFGMaskFrame"];         
        EndFGMaskFrame           =   (int)fs["EndFGMaskFrame"];           
        ApplyMorphologicalFilter =   (int)fs["ApplyMorphologicalFilter"]; 
       
        fs.release();
    }
    else {
        path p("config");
        if (!exists(p))
            create_directory(p);
        
        FileStorage fs(filename, FileStorage::WRITE);
        fs << "History"                  << (int)History;
        fs << "VarThreshold"             << (float)VarThreshold;
        fs << "Alpha"                    << (double)Alpha;
        fs << "InitFGMaskFrame"          << (int)InitFGMaskFrame;
        fs << "EndFGMaskFrame"           << (int)EndFGMaskFrame;
        fs << "ApplyMorphologicalFilter" << (int)ApplyMorphologicalFilter;
        fs.release();

    }

}

void MOG2Builder::GetBackground(OutputArray image) 
{

}

void MOG2Builder::GetForeground(OutputArray mask) 
{

}

void MOG2Builder::Update(InputArray frame, OutputArray mask) 
{
    
    Mat Image = frame.getMat();
    Mat Foreground(Image.size(),CV_8U,Scalar::all(0));
    if (model != NULL) {
        double _alpha = Alpha;
        model->operator()(Image,Foreground,_alpha);
        
        
    }

    // Applying morphological filter
    // Erode the image
    Mat Eroded; 
    if (ApplyMorphologicalFilter) {
        Mat Element(2,2,CV_8U,Scalar(1));
        //erode(Mask,Eroded,Mat());
        erode(Foreground,Eroded,Element);
        Eroded.copyTo(mask);
    }
    else {
        Foreground.copyTo(mask);
    }



}




