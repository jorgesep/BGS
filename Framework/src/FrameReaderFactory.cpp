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
#include "FrameReaderFactory.h"
#include <boost/filesystem.hpp>


#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>



namespace seq {
    
using namespace std;
using namespace boost::filesystem;


ListFiles::ListFiles()
: frame_counter(0),
  delay(25),
  cols(0),
  rows(0),
  nchannels(0)
{

    lookForImageFilesInDirectory(".");
    getImageProperties();

}


ListFiles::ListFiles(string _directory)
: frame_counter(0),
  delay(25),
  cols(0),
  rows(0),
  nchannels(0)
{

    lookForImageFilesInDirectory(_directory);
    getImageProperties();
}

// Look for jpg and png files in directory
void ListFiles::lookForImageFilesInDirectory(string _directory)
{
    boost::filesystem::path _path ( _directory.c_str() );
    if (is_directory(_path)) {
        vector<path> v;
        copy(directory_iterator(_path), directory_iterator(), back_inserter(v));

        for (vector<path>::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it) {
            
            if ( (it->extension() == ".jpg") || (it->extension() == ".png") )
                im_files.push_back((canonical(*it).string()));
        }

        sort(im_files.begin(), im_files.end());

    }

}


void ListFiles::getFrame(OutputArray frame)
{
    Mat Image;

    //if (im_files.size() > 0 && frame_counter < im_files.size() ) {
    //}
    Image = imread(im_files[frame_counter]);
    Image.copyTo(frame);
    frame_counter++;
    
}

void ListFiles::getImageProperties()
{
    if (im_files.size() > 0) {
        
        //create video object.
        Mat Frame = imread(im_files[0]);
        
        // Check file has been opened sucessfully
        if (Frame.data == NULL )
            return ;
        
        cols          = Frame.cols;
        rows          = Frame.rows;
        int frameType = Frame.type();
        nchannels     = CV_MAT_CN(frameType);
    
    }
        
}
    
VideoFile::VideoFile(string filename)
    :  delay(25),
    cols(0),
    rows(0),
    nchannels(0),
    frame_counter(0)

{
    path _path_to_file(filename.c_str());
    if (is_regular_file(_path_to_file))
        video.open(filename.c_str());
    
    videoname = filename;
    getImageProperties();

}

VideoFile::~VideoFile()
{
    if (video.isOpened())
        video.release();
}

void VideoFile::getFrame(OutputArray frame)
{
    Mat Image;
    if (video.isOpened()) {
        video >> Image;
        Image.copyTo(frame);
        frame_counter = video.get(CV_CAP_PROP_POS_FRAMES);
    }

}

void VideoFile::getImageProperties()
{
    VideoCapture video_prop(videoname.c_str());
    if (!video_prop.isOpened())
        return;
    
    Mat Frame;
    video_prop >> Frame;
    
    double rate= video_prop.get(CV_CAP_PROP_FPS);
    delay = 1000/rate ;
    cols = video_prop.get(CV_CAP_PROP_FRAME_WIDTH);
    rows = video_prop.get(CV_CAP_PROP_FRAME_HEIGHT);
    int frameType = Frame.type();
    nchannels = CV_MAT_CN(frameType);

    //reset video to initial position.
    video.set(CV_CAP_PROP_POS_FRAMES, 0);

    video_prop.release();
    
    
}
    
    
FrameReader *FrameReaderFactory :: create_frame_reader(string name)
{
    //Verify input name is a video file or sequences of jpg files
    path _path (name.c_str());
    if (is_directory(_path))
        return new ListFiles(name);
    else if (is_regular_file(_path))
        return new VideoFile(name);
    else
        throw 5;

    return NULL;

}
    

}
