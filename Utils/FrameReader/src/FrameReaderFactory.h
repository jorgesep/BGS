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
#ifndef _FACTORY_READER_H
#define _FACTORY_READER_H

#include <iostream>
#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>


namespace seq 
{
    

using namespace std;
using namespace cv;

class FrameReader
{
public:
    virtual ~FrameReader() {}
    virtual void getFrame (OutputArray) = 0;
    virtual void getFrame (OutputArray, int) = 0;
    virtual int getFrameDelay() = 0;
    virtual int getNChannels() = 0;
    virtual int getNumberCols() = 0;
    virtual int getNumberRows() = 0;
    virtual int getFrameCounter () = 0;
}; 

/** This class encapsulate images from directory
 */ 
class ListFiles : public FrameReader
{
public:
    ListFiles();
    ~ListFiles() {};
    ListFiles(string); 
    virtual void getFrame(OutputArray frame);
    virtual void getFrame(OutputArray frame, int color = 1);
    virtual int getFrameDelay() { return delay; };
    virtual int getNChannels() { return nchannels; };
    virtual int getNumberCols() { return cols; };
    virtual int getNumberRows() { return rows; };
    virtual int getFrameCounter() { return frame_counter; };
private:
    void lookForImageFilesInDirectory(string);
    void getImageProperties();
    int frame_counter;
    
    vector<string> im_files;
    
    int delay;
    int cols;
    int rows;
    int nchannels;
};

class VideoFile : public FrameReader
{
public:
    VideoFile()    :  
    delay(25),
    cols(0),
    rows(0),
    nchannels(0),
    frame_counter(0) {};
    
    ~VideoFile();
    VideoFile(string); 
    virtual void getFrame(OutputArray frame);
    virtual void getFrame(OutputArray frame, int color = 1);
    virtual int getFrameDelay() { return delay; };
    virtual int getNChannels() { return nchannels; };
    virtual int getNumberCols() { return cols; };
    virtual int getNumberRows() { return rows; };
    virtual int getFrameCounter() { return frame_counter; };

private:
    VideoCapture video;
    void getImageProperties();
    int delay;
    int cols;
    int rows;
    int nchannels;
    string videoname;
    int frame_counter;

};

class FrameReaderFactory
{
public:
    static FrameReader* create_frame_reader(string name);
};

}


#endif
