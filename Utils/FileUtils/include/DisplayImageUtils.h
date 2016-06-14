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
//  Special class to register spent times.
//
//  Created by Jorge Sepulveda on 6/28/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _display_images_h
#define _display_images_h
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>
#include "utils.h"

using namespace std;
using namespace cv;

/**
 */
class DisplayImages
{

public:
    // Default constructor , type color images.
    DisplayImages() : type(16){};
    DisplayImages(int rhs) {type=rhs;};
    void mergeImages(InputArray, InputArray, OutputArray);

private:
    int type;
};


#endif
