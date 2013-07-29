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


//
//  mdgkt_filter.h
//  mdgkt_filter
//
//  Created by Jorge Sepulveda on 4/10/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _mdgkt_filter_h
#define _mdgkt_filter_h
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

/**
 * Implementation of spatio-temporal pre-processing filter for
 * smoothing transform.
 */
class mdgkt
{
public:
    static mdgkt* Instance();
    static void deleteInstance();
    void SpatioTemporalPreprocessing(const Mat&, Mat&);
    void initialize();
    void initializeFirstImage(const Mat&);
    void initializeFirstImage(const vector<Mat>&);
    int getTemporalWindow() { return TIME_WINDOW; };

private:
    
    mdgkt() { };
    
    virtual ~mdgkt() { };
    mdgkt(const mdgkt &) { };
    mdgkt& operator=(mdgkt const&){ return *this; };
    
    vector<Mat> kernelImageR;
    vector<Mat> kernelImageG;
    vector<Mat> kernelImageB;

    Mat temporalGaussFilter;
    Mat spatialGaussFilter;

    

    static const unsigned char SPATIO_WINDOW;
    static const unsigned char TIME_WINDOW;
    static const float SIGMA;
    
    static mdgkt* ptrInstance;
    static int numInstances;

    bool has_been_initialized;

};




















#endif
