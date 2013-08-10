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

#include "BGSFramework.h"

Framework::~Framework()
{
    if (_algorithm != NULL) {
        delete _algorithm;
    }
}

void Framework::loadConfigParameters()
{
    if (_algorithm != NULL)
        _algorithm->LoadConfigParameters();
}

void Framework::initializeAlgorithm()
{
    if (_algorithm != NULL)
        _algorithm->Initialization();

}

void Framework::updateAlgorithm(InputArray frame ,OutputArray fgmask)
{
    if (_algorithm != NULL) {
        _algorithm->Update(frame, fgmask);
    }

}
void Framework::getBackground(OutputArray bgframe)
{
    if (_algorithm != NULL) {
        _algorithm->GetBackground(bgframe);
    }

}

void Framework::getForeground(OutputArray fgframe)
{
    if (_algorithm != NULL) {
        _algorithm->GetForeground(fgframe);
    }

}

string Framework::getConfigurationParameters()
{
    string parameters;
    
    if (_algorithm != NULL) {
        parameters = _algorithm->PrintParameters();
    }
    
    return parameters;
    
    
}



