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

#ifndef _NP_ALGORITHM_H
#define _NP_ALGORITHM_H

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "IBGSAlgorithm.h"
#include "NPBGSubtractor.h"

using namespace cv;
using namespace boost::filesystem;


class NPBuilder : public IBGSAlgorithm
{
    
public:
    
    //default constructor
    NPBuilder();
    // parametric constructor
    NPBuilder(int, int, int);
    NPBuilder(Size, int);
    ~NPBuilder();
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

private:
    void loadDefaultParameters();

    int           FramesToLearn;
    int           SequenceLength;
    int           TimeWindowSize;
    unsigned char UpdateFlag;
    unsigned char SDEstimationFlag;
    unsigned char UseColorRatiosFlag;
    double        Threshold;
    double        Alpha;
    unsigned int  InitFGMaskFrame;
    unsigned int  EndFGMaskFrame;
    unsigned char ApplyMorphologicalFilter;
    int rows;
    int cols;
    int nchannels;
    bool has_been_initialized;
    int frame_counter;
    Size frameSize;
    int  frameType;

    unsigned char *FilterFGImage;
    unsigned char **DisplayBuffers;



    static const int           DefaultFramesToLearn;
    static const int           DefaultSequenceLength;
    static const int           DefaultTimeWindowSize;
    static const unsigned char DefaultUpdateFlag;
    static const unsigned char DefaultSDEstimationFlag;
    static const unsigned char DefaultUseColorRatiosFlag;
    static const double        DefaultThreshold;
    static const double        DefaultAlpha;
    static const unsigned int  DefaultInitFGMaskFrame;
    static const unsigned int  DefaultEndFGMaskFrame;
    static const unsigned char DefaultApplyMorphologicalFilter;

    NPBGSubtractor *model;

};


#endif
