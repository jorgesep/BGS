//
//  QualityMetrics.h
//  QualityMetrics
//
//  Created by Jorge Sepulveda on 5/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef Performance_h
#define Performance_h

#include <sstream>
#include "opencv2/video/background_segm.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

namespace bgs {

//Class that keeps all measurement units of quality assesments.
class Performance {
public:
typedef struct ContingencyMatrix
{
    ContingencyMatrix (): tp(0),tn(0),fp(0),fn(0) {};
    ContingencyMatrix(float _tp, float _tn, float _fp, float _fn):
        tp(_tp),tn(_tn),fp(_fp),fn(_fn) {};
    //copy constructor
    ContingencyMatrix (const ContingencyMatrix & rhs) { *this = rhs; };
    //Inequality operator
    bool operator !=(const ContingencyMatrix &rhs) const
    {
        return ((tp!=rhs.tp)||(tn!=rhs.tn)||(fp!=rhs.fp)||(fn!=rhs.fn));
    };
    //equality operator
    bool operator ==(const ContingencyMatrix &rhs) const
    {
        return ((tp==rhs.tp)&&(tn==rhs.tn)&&(fp==rhs.fp)&&(fn==rhs.fn));
    };
    // assigment operator
    ContingencyMatrix &operator =(const ContingencyMatrix &rhs)
    {
        if (*this != rhs) 
        {
            tp = rhs.tp; tn = rhs.tn; fp = rhs.fp; fn = rhs.fn;
        }
        return *this;
    };
    // overloaded += operator
    ContingencyMatrix& operator +=(const ContingencyMatrix &rhs)
    {
       tp += rhs.tp; tn += rhs.tn; fp += rhs.fp; fn += rhs.fn;
       return *this;
    };

    const ContingencyMatrix operator+(const ContingencyMatrix &rhs) const
    {
       //ContingencyMatrix result(rhs.tp,rhs.tn,rhs.fp,rhs.fn);
       ContingencyMatrix result(*this);
       return result += rhs;
    }
/*
    const ContingencyMatrix operator+(const ContingencyMatrix &lhs,const ContingencyMatrix &rhs) const
    {
       ContingencyMatrix result(rhs.tp + lhs.tp,
                      rhs.tn + lhs.tn,
                      rhs.fp + lhs.fp,
                      rhs.fn + lhs.fn);
       return result;
    }

    const ContingencyMatrix operator-(const ContingencyMatrix &lhs,const ContingencyMatrix &rhs)
    {
       ContingencyMatrix result(rhs.tp - lhs.tp,
                      rhs.tn - lhs.tn,
                      rhs.fp - lhs.fp,
                      rhs.fn - lhs.fn);
       return result;
    }
*/
    // overloaded -= operator
    ContingencyMatrix& operator -=(const ContingencyMatrix &rhs) 
    {
        tp -= rhs.tp; tn -= rhs.tn; fp -= rhs.fp; fn -= rhs.fn;
        return *this;
    };

    float tp;
    float tn;
    float fp;
    float fn;
};

typedef struct CommonMetrics {
    CommonMetrics (): sensitivity(0),specificity(0),precision(0) {};
    CommonMetrics(double se, double sp, double pr):
        sensitivity(se),specificity(sp),precision(pr) {};
    //copy constructor
    CommonMetrics (const CommonMetrics & rhs) { *this = rhs; };
    //Inequality operator
    bool operator !=(const CommonMetrics &rhs) const
    {
        return ((sensitivity != rhs.sensitivity)||
                (specificity != rhs.specificity)||
                (precision  != rhs.precision));
    };
    //equality operator
    bool operator ==(const CommonMetrics &rhs) const
    {
        return ((sensitivity == rhs.sensitivity)&&
                (specificity == rhs.specificity)&&
                (precision  == rhs.precision));
    };
    // assigment operator
    CommonMetrics &operator =(const CommonMetrics &rhs)
    {
        if (*this != rhs) 
        {
            sensitivity = rhs.sensitivity; 
            specificity = rhs.specificity; 
            precision   = rhs.precision;
        }
        return *this;
    };
    // overloaded += operator
    CommonMetrics& operator +=(const CommonMetrics &rhs)
    {
        sensitivity += rhs.sensitivity; 
        specificity += rhs.specificity; 
        precision   += rhs.precision;
        return *this;
    };

    const CommonMetrics operator+(const CommonMetrics &rhs) const
    {
       CommonMetrics result(*this);
       return result += rhs;
    }
    // assigment operator
    double sensitivity;
    double specificity;
    double precision;
};

typedef struct GlobalMetrics {
    GlobalMetrics () : 
        perfR(0,0,0,0),
        perfG(0,0,0,0),
        perfB(0,0,0,0),
        metricR(0,0,0),
        metricG(0,0,0),
        metricB(0,0,0),
        count(0) {};
    ContingencyMatrix perfR;
    ContingencyMatrix perfG;
    ContingencyMatrix perfB;
    CommonMetrics metricR;
    CommonMetrics metricG;
    CommonMetrics metricB;
    unsigned int count;
};

public:

    //default constructor
    Performance () : 
        FMeasure(0),Variance(0),Mean(0),threshold(250),
        sensitivity(0),specificity(0),precision(0),nchannel(1)
    { };

    //Parametric constructor
    Performance (float fm, float var, float mu, unsigned char thr, 
            float sn, float sp, float pr, int nch ) :
        FMeasure(fm),Variance(var),Mean(mu),threshold(thr),
        sensitivity(sn),specificity(sp),precision(pr),nchannel(nch)
    { };

    //copy constructor
    Performance (const Performance & rhs) { *this = rhs; };

    // assigment operator
    Performance &operator =(const Performance &rhs)
    {
        if (*this != rhs) 
        {
            FMeasure      = rhs.FMeasure;
            Variance      = rhs.Variance;
            Mean          = rhs.Mean;
            threshold     = rhs.threshold;
            sensitivity   = rhs.sensitivity;
            specificity   = rhs.specificity;
            precision     = rhs.precision;
            nchannel      = rhs.nchannel ;
        }
        return *this;
    };

    //Equality operator
    bool operator ==(const Performance &rhs) const
    {
        return ((FMeasure      == rhs.FMeasure)      &&
                (Variance      == rhs.Variance)      &&
                (Mean          == rhs.Mean)          &&
                (sensitivity   == rhs.sensitivity)   &&
                (specificity   == rhs.specificity)   &&
                (precision     == rhs.precision  )   &&
                (nchannel      == rhs.nchannel   ));
    };

    //Inequality operator
    bool operator !=(const Performance &rhs) const
    {
        return ((FMeasure      != rhs.FMeasure)      ||
                (Variance      != rhs.Variance)      ||
                (Mean          != rhs.Mean)          ||
                (sensitivity   != rhs.sensitivity)   ||
                (specificity   != rhs.specificity)   ||
                (precision     != rhs.precision)     ||
                (nchannel      != rhs.nchannel ));
    };

    // overloaded += operator
    Performance& operator +=(const Performance &rhs)
    {
        FMeasure      += rhs.FMeasure;
        Variance      += rhs.Variance;
        Mean          += rhs.Mean;
        sensitivity   += rhs.sensitivity;
        specificity   += rhs.specificity;
        precision     += rhs.precision  ;
        return *this;
    };

    const Performance operator+(const Performance &rhs) const
    {
        Performance result(rhs.FMeasure,
                rhs.Variance,
                rhs.Mean,
                rhs.threshold,
                rhs.sensitivity,
                rhs.specificity,
                rhs.precision,
                rhs.nchannel);
        return result += rhs;
    }

    // overloaded -= operator
    Performance& operator -=(const Performance &rhs) 
    {
        FMeasure      -= rhs.FMeasure;
        Variance      -= rhs.Variance;
        Mean          -= rhs.Mean;
        sensitivity   -= rhs.sensitivity;
        specificity   -= rhs.specificity;
        precision     -= rhs.precision;
        return *this;
    };

    //Return an string with numbe of the values
    string asString() const;
    string refToString() const;
    string summaryAsString() const;
    string averageSummaryAsString() const;


    /**
     * Compare both image frames at pixel level
     * Results are saved in TP,TN,FP and FN
     */
    void pixelLevelCompare(const Mat&, const Mat&);
    
    /**
     * Compute number of TP and TN.
     */
    void setPixelsReference(const Mat&);
    void setThreshold(int th) {threshold = th;};
    ContingencyMatrix getContingencyMatrix(int idx) {return measure[idx];};

    /**
     * Return performance indexes
     */
    inline float getSensitivity() { return sensitivity;};
    inline float getSpecificity() { return specificity;};
    inline float getPrecision()   { return precision; };
    inline float recall() {return sensitivity;};
    inline float TPR() {return sensitivity; };
    inline float TNR() {return (1 - specificity);};  

    inline float FM() {return FMeasure;};
    inline float Sigma() {return Variance; };
    inline float Mu()  { return Mean;};
   
    float getSensitivity(int);
    float getSpecificity(int);
    float getPrecision(int);

private:
    /**
     * Obtaind perf indexes of the first channel
     */
    void evaluatePerformanceContingencyMatrix();
    void evaluatePerformanceContingencyMatrixPerChannel(int);

    float FMeasure;
    float Variance;
    float Mean;
    float specificity;
    float sensitivity;
    float precision;
    unsigned char threshold;
    unsigned int nchannel;

    static const int MAX_NUMBER_CHANNELS = 3;
    ContingencyMatrix ref[3];
    ContingencyMatrix measure[3];
    GlobalMetrics global_metrics;
    CommonMetrics common_metrics;
};


}
#endif
