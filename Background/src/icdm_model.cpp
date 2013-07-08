//
//  icdm.cpp
//  
//
//  Created by Jorge Sepulveda on 4/13/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "icdm_model.h"

icdm* icdm::ptrInstance = NULL;
int icdm::numInstances = 0;



double icdm::getIlluminationFactor(const Mat& _ref, const Mat& _cur, Method _type)
{
    // initialize
    _factor = 1.0;


    // get Mat header for input image, O(1) operation.
    Mat ref_gray = _ref;
    Mat cur_gray = _cur;

    //brightness or luminance formula: Y=0.299R+0.587G+0.114B
    if (_ref.channels() > 1)
        cvtColor( _ref, ref_gray, CV_BGR2GRAY );
    if (_cur.channels() > 1)
        cvtColor( _cur, cur_gray, CV_BGR2GRAY );

    //remove the pixels which value = 0
    //Mat ref = ref_gray | Mat::ones(ref_gray.size(), ref_gray.type() );
    //ref_gray = ref;

    //ref_gray.convertTo(ref_gray, CV_32F);
    //cur_gray.convertTo(cur_gray, CV_32F);
    
    //int x = ref_gray.rows
    //cout << ref_gray.rows << "X" << ref_gray.cols << endl;

    // Obtain global changes in intensity
    switch (_type)
    {
        case MofQ:
            _factor    = getMedianOfQuotient(ref_gray, cur_gray);
            break;
            
        case aLS:
            cur_gray = cur_gray.mul(cur_gray);
            ref_gray = ref_gray.mul(ref_gray);
            _factor  = sum(cur_gray)[0]/sum(ref_gray)[0];
            break;

        case AofQ:
            {
            Mat global = cur_gray / ref_gray;
            _factor    = sum(global)[0]/global.total();
            break;
            }
        case Ave:
            _factor = sum(cur_gray)[0]/sum(ref_gray)[0];
            break;

        default:
            cout << "Method not correct" << endl;
            break;
    };

    return _factor;

}

double icdm::Median(const Mat& _src)
{
    Mat first_row(_src.row(0));
    std::vector<double> values(first_row.begin<double>(), first_row.end<double>());

    size_t size = values.size();
    double median;

    sort(values.begin(), values.end());

    if( size % 2 == 0 )
    {
        median = (values[size / 2 - 1] + values[size / 2]) / 2;
    }
    else
    {
        median = values[size / 2];
    }

    return median;

}

double icdm::Median(vector<double>& values)
{
    
    size_t size = values.size();
    double median;
    
    sort(values.begin(), values.end());
    
    if( size % 2 == 0 )
    {
        median = (values[size / 2 - 1] + values[size / 2]) / 2;
        //median = (values[size / 2 + 1] + values[size / 2]) / 2;
    }
    else
    {
        median = values[size / 2];
    }
    
    return median;
    
}


double icdm::getMedianOfQuotient(const Mat& _Iref, const Mat& _Icur)
{
    vector<double> values;
    
    
    // get Mat header for input image. This is O(1) operation
    Mat Iref = _Iref;
    if (_Iref.depth() != CV_8U)
        _Iref.convertTo(Iref, CV_8U);
    
    Mat Icur = _Icur;
    if (_Icur.depth() != CV_8U)
        _Icur.convertTo(Icur, CV_8U);

    
    // number of lines
    int nl= Iref.rows; 
    int nc= Iref.cols;
    
    
    if ( Iref.isContinuous() && Icur.isContinuous() ){
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }
    
    
    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        const uchar* _ref_data= Iref.ptr<uchar>(j);
        const uchar* _cur_data= Icur.ptr<uchar>(j);
        
        for (int i=0; i<nc; i++) {

            double num = static_cast<double>(_cur_data[i]);
            double den = _ref_data[i] == 0 ? 1.0 : static_cast<double>(_ref_data[i]);
            double tmp = num/den;
            values.push_back(tmp);
            
        }
    }
    
    return Median(values);
}
/*
 double icdm::getMedianOfQuotient(const Mat& Iref, const Mat& Icur)
 {
 vector<double> values;
 
 //int depth_ref = Iref.depth();
 //int depth_cur = Icur.depth();
 
 //int nchannels = CV_MAT_CN(frameType);
 //CV_Assert( nchannels == 3 );
 
 
 //int type = CV_MAT_DEPTH(<#flags#>)
 
 //Mat global(Iref.size(),CV_32F);
 // number of lines
 int nl= Iref.rows; 
 int nc= Iref.cols;
 
 int t1 = Iref.type();
 int t2 = Icur.type();
 
 if ( Iref.isContinuous() && Icur.isContinuous() ){
 //then no padded pixels
 nc= nc*nl;
 nl= 1; // it is now a 1D array
 }
 
 
 for (int j=0; j<nl; j++) {
 //get a pointer of each row
 const uchar* _ref_data= Iref.ptr<uchar>(j);
 const float* _cur_data= Icur.ptr<float>(j);
 
 for (int i=0; i<nc; i++) {
 
 double num = static_cast<double>(_cur_data[i]);
 double den = _ref_data[i] == 0 ? 1.0 : static_cast<double>(_ref_data[i]);
 double tmp = num/den;
 values.push_back(tmp);
 
 }
 }
 
 return Median(values);
 } 
 
*/
    
    


void icdm::deleteInstance () {
    
    if (ptrInstance) {
        delete ptrInstance;
        ptrInstance = NULL;
    }
    
}

icdm* icdm::Instance() {
    
    if (!ptrInstance) {
        ptrInstance = new icdm();
    }
    //else
    //    cout << "INSTANCE ALREADY CREATED" << endl;
    
    //numInstances++;
    return ptrInstance;
}


