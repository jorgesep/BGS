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

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/regex.hpp>

#include "BGSTimer.h"

BGSTimer* BGSTimer::ptrInstance = NULL;
int BGSTimer::numInstances = 0;


using namespace boost::filesystem;
using namespace boost::posix_time;
using namespace boost::gregorian;


const string BGSTimer::getSequenceElapsedTime()
{
    // construct output line
    // The input contains either directory or video filename
    string name;
    path seq_path( bgs::chomp(sequence_name) );

    if (is_directory(seq_path)) {
        // get last name of directory
        name = regex_replace( seq_path.relative_path().string(), boost::regex("/"), "_")    ;
    }
    else
        // get filename of full path
        name = seq_path.stem().string();


    // Build output line.
    stringstream str;
    str << duration        << " " <<
           start_date_time << " " << stop_date_time  << " " <<
           name            << " " << sequence_parameters ;

    string time_path="timing";
    path p (time_path);
    if ( !exists(p) )
        create_directory(p);

    stringstream ss;
    ss << time_path << "/" << name << ".txt" ;
    string filename = ss.str();

    std::ofstream file;
    file.open (filename.c_str() , ios::out | ios::app );
    file << str.str() << endl;
    file.close();

    const string elapsed = str.str();

    return elapsed;

}

void BGSTimer::resetAll()
{
    sequence_name.clear();
    sequence_parameters.clear();
    start_date_time.clear();
    stop_date_time.clear();

    start_time = 0;
    stop_time = 0;
    duration = 0;

}

void BGSTimer::setSequenceName(string seq_name, string params)
{
    resetAll();

    sequence_name = seq_name;
    sequence_parameters = params;

}

double BGSTimer::getElapsedTime()
{
    double elapsed = 0;
    if (start_time > 0 && stop_time >0)
        elapsed = duration;
    else if (start_time > 0)
        elapsed = (static_cast<double>(cv::getTickCount()) - start_time)/
            cv::getTickFrequency();

    return elapsed;
}


void BGSTimer::registerStartTime()
{
    start_time      = static_cast<double>(cv::getTickCount());

    //get the current time from the clock -- one second resolution
    ptime now = second_clock::local_time();
    start_date_time = to_simple_string(now);

}

void BGSTimer::registerStopTime()
{
    stop_time      = static_cast<double>(cv::getTickCount());

    duration = stop_time - start_time;
    duration /= cv::getTickFrequency(); // time elapsed in ms.

    // Get current date/time, format is YYYY-MM-DD.HH:mm:ss
    ptime now = second_clock::local_time();
    stop_date_time = to_simple_string(now);
}


void BGSTimer::deleteInstance () {
    
    if (ptrInstance) {
        delete ptrInstance;
        ptrInstance = NULL;
    }
    
}

BGSTimer* BGSTimer::Instance() {
    
    if (!ptrInstance) {
        ptrInstance = new BGSTimer();
    }

    return ptrInstance;
}


