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

#ifndef _MOG_ALGORITHM_H
#define _MOG_ALGORITHM_H

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "IBGSAlgorithm.h"

using namespace cv;
using namespace boost::filesystem;


class MOGBuilder : public IBGSAlgorithm
{
    
public:
    
    //default constructor
    MOGBuilder():
        history(100),nmixtures(4),backgroundRatio(0.999),noiseSigma(0),alpha(2.0e-5) {};
    ~MOGBuilder();
    // parametric constructor
    MOGBuilder(int _hist, int _nmix, double _bgRatio, double _noise=0):
        history(_hist),nmixtures(_nmix),backgroundRatio(_bgRatio),noiseSigma(_noise) 
    { };
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
    const string Name() {return string("MOG"); };
    string ElapsedTimeAsString();
    double ElapsedTime() {return duration; } ;

private:
    
    int history;
    int nmixtures;
    double backgroundRatio;
    double noiseSigma;
    double alpha;
    double duration;

    static const int DefaultHistory;
    static const int DefaultNMixtures;
    static const double DefaultBackgroundRatio;
    static const double DefaultNoiseSigma;
    static const double DefaultAlpha;
    
    BackgroundSubtractorMOG *model;

};


#endif
