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
#ifndef _IBGS_ALGORITHM_H
#define _IBGS_ALGORITHM_H

#include <opencv2/opencv.hpp>
using namespace cv;
class IBGSAlgorithm
{
public:
    IBGSAlgorithm() {}
    virtual ~IBGSAlgorithm() {}
    virtual void SetAlgorithmParameters() = 0;
    virtual void LoadConfigParameters()   = 0;
    virtual void SaveConfigParameters()   = 0;
    virtual void Initialization()         = 0;
    virtual void GetBackground(OutputArray)          = 0;
    virtual void GetForeground(OutputArray)          = 0;
    virtual void Update(InputArray, OutputArray)                 = 0;
    virtual void LoadModel()              = 0;
    virtual void SaveModel()              = 0;
    virtual string PrintParameters() = 0;
};


#endif
