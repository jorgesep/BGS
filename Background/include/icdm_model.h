//
//  Illumination-Invariant Change Detection Model
//
//  Created by Jorge Sepulveda on 6/28/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _icdm_model_h
#define _icdm_model_h
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

/**
 * Porting from Matlab
 * Illumination-invariant change detection model (ICDM)
 * Process to identifying illumination variation over time.
 */
class icdm
{
public:
   enum Method
   {
       MofQ = 0,
       aLS,
       AofQ,
       Ave
   };

public:
    static icdm* Instance();
    static void deleteInstance();
    double getIlluminationFactor(const Mat&, const Mat&, Method _type = MofQ);

private:

    icdm() { };

    virtual ~icdm() { };
    icdm(const icdm &) { };
    icdm& operator=(icdm const&){ return *this; };

    double Median(const Mat&);
    double Median(vector<double>&);
    double getMedianOfQuotient(const Mat&, const Mat&);

    static icdm* ptrInstance;
    static int numInstances;

    double _factor;
    Method _method;
};




















#endif
