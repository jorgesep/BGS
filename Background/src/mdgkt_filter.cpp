//
//  mdgkt.cpp
//  sagmm
//
//  Created by Jorge Sepulveda on 4/13/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "mdgkt_filter.h"

const float mdgkt::SIGMA = 0.5;
const unsigned char mdgkt::SPATIO_WINDOW = 3;
const unsigned char mdgkt::TIME_WINDOW = 3;

mdgkt* mdgkt::ptrInstance = NULL;
int mdgkt::numInstances = 0;


// Internal method to initialize to zero all vectors
void mdgkt::initialize() 
{
    // Matrix with gaussian values for processing temporal frames.
    // thia return the following vector [0.106507,0.786986,0.106507]
    temporalGaussFilter = getGaussianKernel(TIME_WINDOW,SIGMA,CV_32F);
    
    spatialGaussFilter= (Mat_<double>(1,9) << 0.0113, 0.0838, 0.0113, 0.0838, 0.6193, 0.0838, 0.0113, 0.0838, 0.0113);
}

void mdgkt::initializeFirstImage(const Mat& img)
{
    //Fills up filter Mats with default values.
    this->initialize();
    
    //Initialize RGB vectors
    for (int i=0; i<SPATIO_WINDOW; i++) {
        kernelImageR.push_back(Mat::zeros(img.size(), CV_32F));
        kernelImageG.push_back(Mat::zeros(img.size(), CV_32F));
        kernelImageB.push_back(Mat::zeros(img.size(), CV_32F));
    }
    
    has_been_initialized = true;

}

void mdgkt::initializeFirstImage(const vector<Mat> & sequences) 
{
    if (sequences.size() > 0) {
        this->initializeFirstImage(sequences.at(0));
    }
    
}


void mdgkt::SpatioTemporalPreprocessing(const Mat& src, Mat& dst)
{
    vector<Mat> nchannels;
    vector<Mat> brg;
    vector<Mat> temporal_average;
    
    
    Mat E;
    src.convertTo(E, CV_32FC3);
    split(E,nchannels);
    
    brg.push_back( Mat::zeros(src.size(), CV_32F));
    brg.push_back( Mat::zeros(src.size(), CV_32F));
    brg.push_back( Mat::zeros(src.size(), CV_32F));
    
        
    //Spatial pre-processing, border lines are zero.
    GaussianBlur(nchannels.at(2), brg.at(2), Size(3,3), 0.5, BORDER_CONSTANT,0);
    GaussianBlur(nchannels.at(1), brg.at(1), Size(3,3), 0.5, BORDER_CONSTANT,0);
    GaussianBlur(nchannels.at(0), brg.at(0), Size(3,3), 0.5, BORDER_CONSTANT,0);
        
    kernelImageR.push_back(brg.at(2));
    kernelImageG.push_back(brg.at(1));
    kernelImageB.push_back(brg.at(0));
    
    //Keep vectors at the same size of SPATIO_WINDOW
    if (kernelImageB.size() > SPATIO_WINDOW )
        kernelImageB.erase(kernelImageB.begin());
    if (kernelImageR.size() > SPATIO_WINDOW )
        kernelImageR.erase(kernelImageR.begin());
    if (kernelImageG.size() > SPATIO_WINDOW )
        kernelImageG.erase(kernelImageG.begin());
    
    //Temporal pre-processing
    for (int i=0; i<TIME_WINDOW; i++) {
        temporal_average.push_back(Mat::zeros(src.size(), CV_32F));        
    }
    
    //Applies sequentially [0.106507,0.786986,0.106507] to each kernel
    const float* tmp_filter = (float *)temporalGaussFilter.data;
    
    for (int i=0; i<TIME_WINDOW; i++) {
        
        temporal_average.at(2)+= kernelImageR.at(i)*tmp_filter[i];
        temporal_average.at(1)+= kernelImageG.at(i)*tmp_filter[i];
        temporal_average.at(0)+= kernelImageB.at(i)*tmp_filter[i];        
    }
    
    merge(temporal_average,dst);

}


void mdgkt::deleteInstance () {
    
    if (ptrInstance) {
        delete ptrInstance;
        ptrInstance = NULL;
    }
    
}

mdgkt* mdgkt::Instance() {
    
    if (!ptrInstance) {
        ptrInstance = new mdgkt();
    }
    else
        cout << "INSTANCE ALREADY CREATED" << endl;
    
    //numInstances++;
    return ptrInstance;
}


