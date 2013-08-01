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

#ifndef _SAGMM_ALGORITHM_H
#define _SAGMM_ALGORITHM_H

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "IBGSAlgorithm.h"

#include "background_subtraction.h"
// These are for spatio temporal filter and
// illumination factor.
#include "mdgkt_filter.h"
#include "icdm_model.h"

using namespace cv;
using namespace boost::filesystem;

class SAGMMBuilder : public IBGSAlgorithm
{
    
public:
    
    //default constructor
    SAGMMBuilder();
    ~SAGMMBuilder();
    void SetAlgorithmParameters()  {};
    void LoadConfigParameters();
    void SaveConfigParameters() {};
    void Initialization();
    void GetBackground(OutputArray);
    void GetForeground(OutputArray);
    void Update(InputArray, OutputArray);
    void LoadModel() {};
    void SaveModel() {};

private:

    int GaussiansNo;
    double Sigma;
    double SigmaMax;
    double SigmaMin;
    double Cf;
    double Range;
    double Gen;
    double CT;
    double Tau;
    double Alpha;
    unsigned int  InitFGMaskFrame;
    unsigned int  EndFGMaskFrame;
    unsigned char ApplyMorphologicalFilter;
    
    static const int    DefaultGaussiansNo;
    static const double DefaultSigma;
    static const double DefaultSigmaMax;
    static const double DefaultSigmaMin;
    static const double DefaultCf;
    static const double DefaultRange;
    static const double DefaultGen;
    static const double DefaultCT;
    static const double DefaultTau;
    static const double DefaultAlpha;
    static const unsigned int  DefaultInitFGMaskFrame;
    static const unsigned int  DefaultEndFGMaskFrame;
    static const unsigned char DefaultApplyMorphologicalFilter;


    int frame_counter;
    bool has_been_initialized;
    bool update_bg_model;
    Mat Background;


    BackgroundSubtractorMOG3 *model;
    mdgkt *filter;
    icdm  *factor;



};


#endif
