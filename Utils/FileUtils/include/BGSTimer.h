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

#ifndef _timer_h
#define _timer_h
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
class BGSTimer
{

public:
    static BGSTimer* Instance();
    static void deleteInstance();
    void registerStartTime();
    void registerStopTime();
    void setSequenceName(string, string);
    string getSequenceTime(string);
    double getElapsedTime();
    const string getSequenceElapsedTime();

private:

    BGSTimer() { };

    virtual ~BGSTimer() { };
    BGSTimer(const BGSTimer &) { };
    BGSTimer& operator=(BGSTimer const&){ return *this; };

    void resetAll();

    static BGSTimer* ptrInstance;
    static int numInstances;

    string sequence_name;
    string sequence_parameters; 
    double duration;
    double start_time;
    double stop_time;
    string start_date_time;
    string stop_date_time;
};




















#endif
