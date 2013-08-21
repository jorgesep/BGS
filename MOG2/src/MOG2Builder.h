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

#ifndef _MOG2_ALGORITHM_H
#define _MOG2_ALGORITHM_H

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "IBGSAlgorithm.h"

using namespace cv;
using namespace boost::filesystem;


class MOG2Builder : public IBGSAlgorithm
{
    
public:
    
    //default constructor
    MOG2Builder();
    ~MOG2Builder();
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
   
    int History;
    float VarThreshold;
    bool ShadowDetection;
    double LearningRate;
    unsigned int  InitFGMaskFrame;
    unsigned int  EndFGMaskFrame;
    unsigned char ApplyMorphologicalFilter;

    static const int DefaultHistory;
    static const float DefaultVarThreshold;
    static const bool DefaultShadowDetection;
    static const double DefaultLearningRate;
    static const unsigned int  DefaultInitFGMaskFrame;
    static const unsigned int  DefaultEndFGMaskFrame;
    static const unsigned char DefaultApplyMorphologicalFilter;
    
    BackgroundSubtractorMOG2 *model;

};


#endif
