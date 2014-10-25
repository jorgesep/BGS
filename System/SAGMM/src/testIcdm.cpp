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

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sstream>
#include <map>

//#include "bgs.h"
#include "icdm_model.h"
//#include "background_subtraction.h"
//#include "Performance.h"
//#include "utils.h"

using namespace cv;
using namespace std;


int main( int argc, char** argv )
{

    
    icdm* factor = icdm::Instance();
    //Mat current = imread("/Users/jsepulve/Downloads/BGS/Performance/config/bg11.png");
    //Mat reference   = imread("/Users/jsepulve/Downloads/BGS/Performance/config/bg22.png");

    //Mat reference = imread("/Users/jsepulve/Downloads/BGS/build/config/ref.jpg", CV_LOAD_IMAGE_COLOR);
    //Mat current   = imread("/Users/jsepulve/Downloads/BGS/build/config/cur.jpg", CV_LOAD_IMAGE_COLOR);
    //Mat reference = imread("/Users/jsepulve/Downloads/BGS/build/config/reference.jpg", CV_LOAD_IMAGE_COLOR);
    //Mat current   = imread("/Users/jsepulve/Downloads/BGS/build/config/test1.jpg", CV_LOAD_IMAGE_COLOR);
    

    Mat reference = imread("/Users/jsepulve/Downloads/BGS/build/bg_1.png", CV_LOAD_IMAGE_COLOR);
    Mat current   = imread("/Users/jsepulve/Downloads/BGS/build/bg_2.png", CV_LOAD_IMAGE_COLOR);

    float _factor = factor->getIlluminationFactor(reference, current);
    
    cout << "FACTOR: " << _factor << endl;
 
    return 0;
}

