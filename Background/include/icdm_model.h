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
//  Illumination-Invariant Change Detection Model
//
//  Created by Jorge Sepulveda on 6/28/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _icdm_model_h
#define _icdm_model_h
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

/**
 * Porting from Matlab
 * Illumination-invariant change detection model (ICDM)
 * Process to identifying illumination variation over time.
 */
class icdm
{
public:
   enum Method
   {
       MofQ = 0,
       aLS,
       AofQ,
       Ave
   };

public:
    static icdm* Instance();
    static void deleteInstance();
    double getIlluminationFactor(const Mat&, const Mat&, Method _type = MofQ);

private:

    icdm() { };

    virtual ~icdm() { };
    icdm(const icdm &) { };
    icdm& operator=(icdm const&){ return *this; };

    double Median(const Mat&);
    double Median(vector<double>&);
    double getMedianOfQuotient(const Mat&, const Mat&);

    static icdm* ptrInstance;
    static int numInstances;

    double _factor;
    Method _method;
};




















#endif
