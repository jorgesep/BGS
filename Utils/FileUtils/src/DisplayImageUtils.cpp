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
#include <math.h>

#include "DisplayImageUtils.h"

void DisplayImages::mergeImages(InputArray _im1, InputArray _im2, OutputArray img)
{
    // Prepare
    if (_im1.getMat().depth() == type)
        cout << "";

    vector<Mat> Images;
    Images.push_back(_im1.getMat());
    Images.push_back(_im2.getMat());
    //int numberImages = 2;
    
    if ( Images[0].empty() || Images[1].empty() ) return;


    // w - Maximum number of images in a row
    // h - Maximum number of images in a column
    int w = 2;
    int h = 1;
    int size = 300;


    // Create a new 1 channel image
    img.create(Size(60 + size*w, 5 + size*h),CV_8UC3);
    Mat mask = img.getMat();
    mask     = Scalar::all(255);

    int m = 20;
    int n = 20;
    for (vector<Mat>::iterator it = Images.begin() ; it != Images.end(); ++it) {


        //Find the width and height of the image
        int x = it->cols;
        int y = it->rows;
        
        // Find whether height or width is greater in order to resize the image
        int max = (x > y)? x: y;
        
        // Find the scaling factor to resize the image
        float scale = (float) ( (float) max / size );
        
        // Used to Align the images
        //if( i % w == 0 && m!= 20) {
        //    m = 20;
        //    n+= 20 + size;
        //}
        
    
        //Make a rectangle
        Rect roi( m, n, (int)( x/scale ), (int)( y/scale ) );
       
        // Set the image ROI to display the current image
        //Point a cv::Mat header at it (no allocation is done)
        Mat ImageROI = mask(roi);
        
        // Resize the input image and copy the it to the Single Big Image
        resize(*it, ImageROI, ImageROI.size(), 0, 0);
        
        m += (20 + size);
        
    }

}


ChunkImage::ChunkImage(InputArray src, int num):sWidth(0,0,0), sHeight(0,0,0)
{
    Mat Image=src.getMat();
    nrows = Image.rows;
    ncols = Image.cols;

    size = Size(ncols,nrows);

    sWidth = computeSubImageSize(num, ncols);
    sHeight = computeSubImageSize(num, nrows);
    nrows = sHeight.len;
    ncols = sWidth.len;
    numSubImages = num;
}

ChunkImage::ChunkImage(int row, int col, int num):sWidth(0,0,0), sHeight(0,0,0)
{
    size = Size(col,row);

    sWidth = computeSubImageSize(num, col);
    sHeight = computeSubImageSize(num, row);
    nrows = sHeight.len;
    ncols = sWidth.len;
    numSubImages = num;
}

void ChunkImage::mergeImages(const vector<Mat>& src, OutputArray dst)
{

    // Create a new 1 channel image
    dst.create(size,src[0].type());
    Mat mask = dst.getMat();
    mask     = Scalar::all(255);

    int x = 0;
    int y = 0;
    vector<Mat>::const_iterator it;
    for (it = src.begin() ; it != src.end(); ++it) {


        //Find the width and height of the image
        int wlen = it->cols;
        int hlen = it->rows;

        // Debugging
       
        /*
        std::cout << "ChunkImage::mergeImages Image[" 
              << x << ":" << y << ":" << wlen << ":" << hlen << ":" 
              << size.height << ":" << size.width << "]\n";
        */

        //Make a rectangle
        Rect roi( x, y, wlen, hlen);
        //Mat subImg (Image, Rect(x, y, wlen, hlen) ); 
       
        // Set the image ROI to display the current image
        //Point a cv::Mat header at it (no allocation is done)
        Mat ImageROI = mask(roi);
        
        // Resize the input image and copy the it to the Single Big Image
        resize(*it, ImageROI, ImageROI.size(), 0, 0);

        x += wlen;
        if (x == size.width) {
            y +=hlen;
            x = 0;
        }
        
    }


}

void ChunkImage::operator()(InputArray src, vector<Mat>& Output) 
{
    Mat Image=src.getMat();

    /*
    std::cout << "ChunkImage::operator Image[" 
              << Image.type() << ":" << Image.depth() << ":" << Image.size() << "]\n";
    std::cout << "ChunkImage::operator SubImage[" 
              << ncols << ":" << nrows 
              << " SubImgLen[" << sWidth.len << ":" << sWidth.rest
              << ":" << sHeight.len <<":"<< sHeight.rest << "]\n";
    */

    int x = 0;
    int y = 0;
    int wlen = sWidth.len;
    int hlen = sHeight.len;
    for (int row=0; row<sHeight.partition; row++) {

        for (int col=0; col<sWidth.partition; col++) {

            /* 
            std::cout <<  "ChunkImage::operator row:col[" 
                      << row << ":" << col << "] y:x [" << y << ":" << x << "]\n";
            */

            if( (x+wlen) == (col*wlen) ) wlen += sWidth.rest;

            Mat subImg (Image, Rect(x, y, wlen, hlen) ); 
            Output.push_back(subImg);

            x += wlen;
            

        }

        y += hlen;
        x = 0;
        if ( y == (row*hlen) ) {
            hlen += sHeight.rest;
        }
    }
    //numSubImages = Output.size();
};

ChunkImage::block ChunkImage::computeSubImageSize(int num, int len) {

    int result   = int(sqrt(float(num))) ;
    int division = int(len / result);
    int residual = int(len % result); 
    return ChunkImage::block(division,residual,result);

}

