/*******************************************************************************
 * <Self-Adaptive Gaussian Mixture Model.>
 * Copyright (C) <2013>  <name of author>
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
#include <opencv2/opencv.hpp>
#include "utils.h"

using namespace cv;
using namespace std;

namespace bgs {

// Remove slash in last position of string.
string chomp (string dir)
{
    unsigned found = dir.find_last_of("/");
    if (found == dir.size()-1) return dir.substr(0,found);
    return dir;
}

unsigned int name_to_number(string file)
{
    unsigned int sn;
    string gt ("-");
    string str(".");
    //Remove ".png"
    size_t pos  = file.find(gt);
    size_t size = file.substr(pos+1).find_last_of(str);
    string number = file.substr(pos+1,size);
    stringstream tmpstr(number);
    tmpstr >> sn;
    return sn;
}
  
void list_files(string directory, map<unsigned int,string>& list, string type)
{
    DIR *dir;
    struct dirent *entity;

    dir = opendir(directory.c_str());
    if (dir != NULL){
        while ( (entity = readdir(dir)) ){
            const string file_name = entity->d_name;
            const string full_file_name = chomp(directory) + "/" + file_name;

            if(entity->d_type == DT_DIR)
                continue;
            if(entity->d_type == DT_REG) {
                //size_t found = file_name.find(".PNG");
                size_t found = file_name.find(type);
                if (found!=std::string::npos)
                    list[name_to_number(entity->d_name)]=full_file_name; 
           }
        }
    }
}

Point stringToPoint(string _point)
{
    if (_point.empty())
        return Point(0,0);

    size_t pos = _point.find(',');
    size_t end = _point.size() - pos;

    stringstream X1( _point.substr(0,pos) );
    stringstream Y1( _point.substr(pos+1,end) );
    int x;
    int y;
    X1 >> x;
    Y1 >> y;

    return Point(x,y);

}

    
void showMultipleImages()
{
    Point pt = Point(1,1);
    //cout << "HOLA" << endl;
}

    
bool FileExists( const char* FileName )
{
    DIR *dir;
    struct dirent *entity;
    dir = opendir(FileName);
    if (dir != NULL){
        entity = readdir(dir);
        if(entity->d_type == DT_DIR)
            return false;
        if(entity->d_type == DT_REG)
           return true;

    }
    return true;

    /*
    FILE* fp = NULL;
        
    fp = fopen( FileName, "rb" );
    if( fp != NULL )
    {
        fclose( fp );
        return true;
    }       
    return false;
    */
}    
    

}
