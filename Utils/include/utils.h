#ifndef bgs_utils_h
#define bgs_utils_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>
#include <dirent.h>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

namespace bgs {

// Remove last position slash of input string.
string chomp (string dir);

// Convert name of ground-truth files in a number identifier.
unsigned int name_to_number(string file);

//Return a list of PNG files from ground-truth directory
void list_files(string directory, map<unsigned int,string>& list, string type = ".PNG");

//Return a a point (x,y) from string 'x,y'
Point stringToPoint(string);

//This function show four images in one window.
void showMultipleImages();

bool FileExists( const char* );

}
#endif
