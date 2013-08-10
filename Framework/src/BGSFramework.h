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
#ifndef _BGS_FRAMEWORK_H
#define _BGS_FRAMEWORK_H

#include <opencv2/opencv.hpp>
#include "IBGSAlgorithm.h"

using namespace cv;

class Framework
{
public:
    Framework() : _algorithm(0) {};
    ~Framework();

    void setAlgorithm(IBGSAlgorithm *_alg)
    {
        _algorithm = _alg;
    };

    void loadConfigParameters();
    void initializeAlgorithm();
    void updateAlgorithm(InputArray,OutputArray);
    void getBackground(OutputArray);
    void getForeground(OutputArray);
    void setName(string _name) { name = _name; };
    string getName() { return name; };
    string getConfigurationParameters();

private:
    IBGSAlgorithm *_algorithm;
    string name;
};


#endif


