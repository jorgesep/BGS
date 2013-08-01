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
#include "MOGBuilder.h"


const int    MOGBuilder::DefaultHistory         = 500;
const int    MOGBuilder::DefaultNMixtures       = 4;
const double MOGBuilder::DefaultBackgroundRatio = 0.999;
const double MOGBuilder::DefaultNoiseSigma      = 0;
const double MOGBuilder::DefaultAlpha           = 0.0001;



MOGBuilder::~MOGBuilder()
{
//    if (bg_model != NULL) {
//        delete bg_model;
//    }
//    bg_model = 0;
}

void MOGBuilder::Initialization()
{
    model = new BackgroundSubtractorMOG(history, nmixtures, backgroundRatio, noiseSigma);

    
    
}

void MOGBuilder::LoadConfigParameters()
{
    string filename = "config/mog.xml";

    if ( exists(filename) && is_regular_file(filename) ) {
        
        FileStorage fs(filename, FileStorage::READ);
        
        history         = (int)fs["History"];
        nmixtures       = (int)fs["NMixtures"];
        backgroundRatio = (double)fs["BackgroundRatio"];
        noiseSigma      = (double)fs["NoiseSigma"];
        alpha           = (double)fs["Alpha"];
        
        fs.release();
    }
    else {
        path p("config");
        if (!exists(p))
            create_directory(p);
        
        FileStorage fs(filename, FileStorage::WRITE);
        fs << "History" << history;
        fs << "NMixtures" << nmixtures;
        fs << "BackgroundRatio" << backgroundRatio;
        fs << "NoiseSigma" << noiseSigma;
        fs << "Alpha" << alpha;
        fs.release();

    }

}

void MOGBuilder::GetBackground(OutputArray image) 
{

}

void MOGBuilder::GetForeground(OutputArray mask) 
{

}

void MOGBuilder::Update(InputArray frame, OutputArray mask) 
{
    
    Mat Image = frame.getMat();
    Mat Foreground(Image.size(),CV_8U,Scalar::all(0));
    if (model != NULL) {
        double _alpha = alpha;
        model->operator()(Image,Foreground,_alpha);
        
        
    }
    Foreground.copyTo(mask);

}




