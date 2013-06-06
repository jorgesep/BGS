/*******************************************************************************
 * <Self-Adaptive Gaussian Mixture Model.>
 * Copyright (C) <2013>  <name of author>
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

#include <stdio.h>
#include <opencv2/opencv.hpp>

#include <iomanip> 

#include "QualityMeasurementUnits.h"

#include <iostream>
#include <fstream>



using namespace cv;
using namespace std;
using namespace bgs;



int main( int argc, char** argv )
{
    Mat im    = imread("input.png",1);
    Mat gt    = imread("input1.png",1);


    MeasurementUnits *units = new MeasurementUnits(); 
    delete units;
    
    //quality quality_metric();
    //
    ////configure foreground and background pixel
    //// in case of color picture takes Blue
    //quality_metric->setmask(image);
    //quality_metric->compare(image,image2);
    //values = quality_metrics->getMetrics();
    //cout << quality_metrics->printLastMetrics << endl;
    //cout << quality_metrics->printSummaryMetrics << endl;
    //getVariance()
    //getMean();
    //
    
    

    return 0;
}

