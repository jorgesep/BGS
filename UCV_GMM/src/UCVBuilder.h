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

#ifndef _UCV_ALGORITHM_H
#define _UCV_ALGORITHM_H

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "IBGSAlgorithm.h"

//#include "ucv_types.h"
//#include "ucv_gmm_data.h"
//#include "ucv_gmm_d.h"
//#include "ucv_gmm.h"

#include "gmm_line.h"
#include "gmm_sample.h"
#include "gmm_double_ucv.h"



using namespace cv;
using namespace boost::filesystem;


class UCVBuilder : public IBGSAlgorithm
{
    
public:

    //default constructor
    UCVBuilder();
    // parametric constructor
    UCVBuilder(int, int, int);
    UCVBuilder(Size, int);
    ~UCVBuilder();
    void SetAlgorithmParameters()  {};
    void LoadConfigParameters();
    void SaveConfigParameters() {};
    void Initialization();
    void GetBackground(OutputArray);
    void GetForeground(OutputArray);
    void Update(InputArray, OutputArray);
    void LoadModel() {};
    void SaveModel() {};
    string PrintParameters();
    const string Name() {return string("UCV"); }; 
    string ElapsedTimeAsString();
    double ElapsedTime() {return duration; };

private:

    void loadDefaultParameters();
   
    float         Threshold;
    double        LearningRate;
    unsigned int  InitFGMaskFrame;
    unsigned int  NumberGaussians;
    unsigned int  EndFGMaskFrame;
    unsigned char ApplyMorphologicalFilter;
    int           TypeFunction;
    unsigned int  cols;
    unsigned int  rows;
    unsigned int  nchannels;
    unsigned int  len;
    double duration;

    bool has_been_initialized;
    int frame_counter;
    Size frameSize;
    int  frameType;

    // Define special variables for UCV GMM
    unsigned char *current_frame_ptr;
    ucv_image_t current_frame;


    // Define constants
    static const double        DefaultThreshold;
    static const double        DefaultLearningRate;
    static const unsigned int  DefaultInitFGMaskFrame;
    static const unsigned int  DefaultEndFGMaskFrame;
    static const unsigned int  DefaultApplyMorphologicalFilter;
    static const unsigned int  DefaultTypeFunction;
    static const unsigned int  DefaultNumberGaussians;

    static const unsigned int  DefaultColumns;
    static const unsigned int  DefaultRows;
    static const unsigned int  DefaultNumberChannels;

    gmm_line   *linear;
    gmm_sample *staircase;
    gmm_double_ucv *gmm;

};


#endif
