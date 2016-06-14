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


