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

#include <stdio.h>
#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include <fstream>

//
//#include "Performance.h"
#include "utils.h"

#include "ucv_types.h"
#include "ucv_gmm_data.h"
#include "ucv_gmm_d.h"
#include "ucv_gmm.h"



using namespace cv;
using namespace std;
//using namespace boost::filesystem;
//using namespace seq;
//using namespace bgs;



void create_g_img(ucv_image_t *a, uint8_t *a_b, uint16_t w, uint16_t h)
{
    a->image = a_b;
    a->type = UCV_GRAY8U;
    a->width = w;
    a->height = h;
}



int main( int argc, char** argv )
{

  
    // Reading input single frame just for testing.
    Mat input_image = imread("00000001.jpg");

    ucv_image_t *im;
    
    // initialize image struct.
    create_g_img(im, (uint8_t*)input_image.data, input_image.cols, input_image.rows);
    

    // Initializing gmm gaussian.
    uint32_t number_gaussians  = 4;
    uint32_t tmp = UCV_IMAGE_WIDTH(*im) * UCV_IMAGE_HEIGHT(*im) * number_gaussians;
    ucv_gmm_gaussian_t *data = new ucv_gmm_gaussian_t[tmp];

    
    
    // gaussian model
    ucv_gaussian_model_t b;
    ucv_gaussian_d_t *data1 = new ucv_gaussian_d_t;
    
    b.bgnd   = data1; 
    b.type   = UCV_GRAY8U;
    
    b.width  = UCV_IMAGE_WIDTH(*im);
    b.height = UCV_IMAGE_HEIGHT(*im);
    b.n_g    = number_gaussians;

    // Initialize threashold, learning rate, mu and sigma.
    ucv_gmm_param_t p;

    uint8_t th = 0.999;
    real_t  lr = 0.002;

    p.th = th;
    p.lr = lr;
   
    p.n_mu    = ucv_gmm_compute_n_mu(lr);
    p.n_sigma = ucv_gmm_compute_n_sigma(lr);
       
    
    
    if (ucv_gmm_sample_init(im, &b, &p) < 0) {
        cout <<  "Impossibile to init bgnd" << endl;
        return -1;
    }

    ucv_image_t mask;
    mask.type = UCV_BWBIN;
    mask.width = UCV_IMAGE_WIDTH(*im);
    mask.height = UCV_IMAGE_HEIGHT(*im);
    tmp = UCV_IMAGE_WIDTH(*im) * UCV_IMAGE_HEIGHT(*im) / 8;
    uint8_t * mask_b = new uint8_t[tmp];
    mask.image = mask_b;

    
    //Main loop.
    
    int8_t tmp = ucv_gmm_sample_update(im, &b, &mask);

    return 0;
}

