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

#include <list>
#include <regex.h>

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

string fileName (const string dir) 
{
    string _dir = chomp(dir);
    unsigned found = _dir.find_last_of("/");
    if (found != string::npos )
        return _dir.substr(found+1,_dir.size());
    return _dir;
        
}
    
    
unsigned int name_to_number(string file)
{
    
    //std::regex e ("-|_|.");
    unsigned int sn;
    string gt ("-");
    string str(".");
    //Remove ".png"
    size_t pos  = file.find(gt);
    
    if (pos == string::npos) {
        pos = -1;
        pos = file.find("_");
    }
    
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


void find_dir_by_name(string name, map<unsigned int,string>& list)
{
    DIR *dir;
    struct dirent *entity;
    
    std::list<string> _list;
    std::list<std::string>::iterator it;
    
    dir = opendir(".");
    if (dir != NULL){
        while ( (entity = readdir(dir)) ){
            const string file_name = entity->d_name;
            //const string full_file_name = chomp(directory) + "/" + file_name;
            if(entity->d_type == DT_REG)
                continue;
            if(entity->d_type == DT_DIR){
                size_t found = file_name.find(name);
                if (found!=std::string::npos)
                    _list.push_back(entity->d_name);
            }
        }
    }
    if (_list.size() > 0 ) {
        _list.sort();
        int i = 1;
        for (it=_list.begin(); it!=_list.end(); ++it) {
            list[i++] = *it;
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
    return false;

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
    

bool DirectoryExists( const char* path )
{
    if ( path == NULL) return false;
    
    DIR *pDir;
    bool bExists = false;
    
    pDir = opendir (path);
    
    if (pDir != NULL)
    {
        bExists = true;    
        (void) closedir (pDir);
    }
    
    return bExists;
}    
    
void CreateDirectory( const char* path) 
{
    map<unsigned int,string> maplist;
    
    if (DirectoryExists(path)) {
        find_dir_by_name(path,maplist);
        map<unsigned int, string>::iterator it = maplist.end();
        it--;
        int number = (*it).first;
        
        stringstream str;
        
        str << path << "." << number;
        cout << "Moving directory " << path << " to " << str.str() << endl;

        rename(path, str.str().c_str());
    }
    
    mkdir(path,S_IRWXU|S_IRGRP|S_IXGRP);
}
    
}
