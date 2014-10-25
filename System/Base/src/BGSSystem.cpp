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

#include "BGSSystem.h"

BGSSystem::~BGSSystem()
{
    if (_algorithm != NULL) {
        delete _algorithm;
    }
}

void BGSSystem::loadConfigParameters()
{
    if (_algorithm != NULL)
        _algorithm->LoadConfigParameters();
}

void BGSSystem::initializeAlgorithm()
{
    if (_algorithm != NULL)
        _algorithm->Initialization();

}

void BGSSystem::updateAlgorithm(InputArray frame ,OutputArray fgmask)
{
    if (_algorithm != NULL) {
        _algorithm->Update(frame, fgmask);
    }

}
void BGSSystem::getBackground(OutputArray bgframe)
{
    if (_algorithm != NULL) {
        _algorithm->GetBackground(bgframe);
    }

}

void BGSSystem::getForeground(OutputArray fgframe)
{
    if (_algorithm != NULL) {
        _algorithm->GetForeground(fgframe);
    }

}

string BGSSystem::getConfigurationParameters()
{
    string parameters;
    
    if (_algorithm != NULL) {
        parameters = _algorithm->PrintParameters();
    }
    
    return parameters;
    
    
}



