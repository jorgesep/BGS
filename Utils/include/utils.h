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
#ifndef bgs_utils_h
#define bgs_utils_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>

//#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

namespace bgs {

struct NPBGConfig 
{
    unsigned int  FramesToLearn;
    unsigned int  SequenceLength;
    unsigned int  TimeWindowSize;
    unsigned char SDEstimationFlag;
    unsigned char UseColorRatiosFlag;
    double         Threshold1;
    double         Threshold2;
    double         Alpha;

    NPBGConfig ():
        FramesToLearn(0),
        SequenceLength(0),
        TimeWindowSize(0),
        SDEstimationFlag(0),
        UseColorRatiosFlag(0),
        Threshold1(0),
        Threshold2(0),
        Alpha(0) {};

    NPBGConfig (unsigned int _f, unsigned int _s, unsigned int _t, unsigned char _sf, unsigned char _uf, double _t1, double _t2, double _a):
        FramesToLearn(_f),
        SequenceLength(_s),
        TimeWindowSize(_t),
        SDEstimationFlag(_sf),
        UseColorRatiosFlag(_uf),
        Threshold1(_t1),
        Threshold2(_t2),
        Alpha(_a) {};
    NPBGConfig (const NPBGConfig & rhs) { *this = rhs; };
    bool operator != (const NPBGConfig & rhs) const
    {
        return (
        (FramesToLearn      != rhs.FramesToLearn     ) ||
        (SequenceLength     != rhs.SequenceLength    ) ||
        (TimeWindowSize     != rhs.TimeWindowSize    ) ||
        (SDEstimationFlag   != rhs.SDEstimationFlag  ) ||
        (UseColorRatiosFlag != rhs.UseColorRatiosFlag) ||
        (Threshold1         != rhs.Threshold1        ) ||
        (Threshold2         != rhs.Threshold2        ) ||
        (Alpha              != rhs.Alpha    ) );

    };
    bool operator == (const NPBGConfig & rhs) const
    {
        return (
        (FramesToLearn      == rhs.FramesToLearn     ) &&
        (SequenceLength     == rhs.SequenceLength    ) &&
        (TimeWindowSize     == rhs.TimeWindowSize    ) &&
        (SDEstimationFlag   == rhs.SDEstimationFlag  ) &&
        (UseColorRatiosFlag == rhs.UseColorRatiosFlag) &&
        (Threshold1         == rhs.Threshold1        ) &&
        (Threshold2         == rhs.Threshold2        ) &&
        (Alpha              == rhs.Alpha    ) );

    };
    NPBGConfig &operator =(const NPBGConfig & rhs)
    {
        if (*this != rhs) 
        {
        FramesToLearn      = rhs.FramesToLearn     ;
        SequenceLength     = rhs.SequenceLength    ;
        TimeWindowSize     = rhs.TimeWindowSize    ;
        SDEstimationFlag   = rhs.SDEstimationFlag  ;
        UseColorRatiosFlag = rhs.UseColorRatiosFlag;
        Threshold1         = rhs.Threshold1        ;
        Threshold2         = rhs.Threshold2        ;
        Alpha              = rhs.Alpha             ;
        }
        return *this;
    };
};
    
string get_current_path();

// Remove last position slash of input string.
string chomp (string dir);
    
// Remove slash and return file name.
string fileName(const string dir);

// Convert name of ground-truth files in a number identifier.
unsigned int name_to_number(string file);

//Return a list of PNG files from ground-truth directory
void list_files(string directory, map<unsigned int,string>& list, string type = ".PNG");

// Look for jpg or png  file  in directory  and return  a list of them
void find_list_any_image_file(string directory, map<unsigned int,string>& list);
void find_dir_by_name(string , map<unsigned int,string>& );

void create_foreground_directory(string& path);

void parse_file(string filename, vector< pair<string, string> >& pair_str);
    
    
//Return a a point (x,y) from string 'x,y'
Point stringToPoint(string);

//This function show four images in one window.
void showMultipleImages();

bool FileExists( const char* );
bool DirectoryExists( const char* path );
void CreateDirectory( const char* path);
void ListDirectories( string, vector<string> &);
void ListImgFilesInDirectory( string, vector<string> &);
void MapAllMasksDirectories( string, map<int,map<int,string> >&);
void MapImgFilesInDirectory( string, map<int,string>&);
    
NPBGConfig* loadInitParametersFromFile(string init="config/np_init.txt");
}
#endif
