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

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace cv;
using namespace std;

namespace bgs {

//Class that keeps all measurement units of quality assesments.
class Performance {
public:
//typedef struct ContingencyMatrix
struct ContingencyMatrix
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

//typedef struct CommonMetrics {
struct CommonMetrics {
    CommonMetrics (): sensitivity(0),specificity(0),MCC(0) {};
    CommonMetrics(double se, double sp, double pr):
        sensitivity(se),specificity(sp),MCC(pr) {};
    //copy constructor
    CommonMetrics (const CommonMetrics & rhs) { *this = rhs; };
    //Inequality operator
    bool operator !=(const CommonMetrics &rhs) const
    {
        return ((sensitivity != rhs.sensitivity)||
                (specificity != rhs.specificity)||
                (MCC         != rhs.MCC));
    };
    //equality operator
    bool operator ==(const CommonMetrics &rhs) const
    {
        return ((sensitivity == rhs.sensitivity)&&
                (specificity == rhs.specificity)&&
                (MCC         == rhs.MCC));
    };
    // assigment operator
    CommonMetrics &operator =(const CommonMetrics &rhs)
    {
        if (*this != rhs) 
        {
            sensitivity = rhs.sensitivity; 
            specificity = rhs.specificity; 
            MCC         = rhs.MCC;
        }
        return *this;
    };
    // overloaded += operator
    CommonMetrics& operator +=(const CommonMetrics &rhs)
    {
        sensitivity += rhs.sensitivity; 
        specificity += rhs.specificity; 
        MCC         += rhs.MCC;
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
    double MCC;
};

//typedef struct GlobalMetrics {
struct GlobalMetrics {
    GlobalMetrics () : 
        perfR(0,0,0,0),
        perfG(0,0,0,0),
        perfB(0,0,0,0),
        metricR(0,0,0),
        metricG(0,0,0),
        metricB(0,0,0),
        count(0) {};

    GlobalMetrics( ContingencyMatrix pR, 
                   ContingencyMatrix pG, 
                   ContingencyMatrix pB,
                   CommonMetrics     mR, 
                   CommonMetrics     mG, 
                   CommonMetrics     mB, 
                   unsigned int cn):
                   perfR(pR),   
                   perfG(pG),   
                   perfB(pB),
                   metricR(mR), 
                   metricG(mG), 
                   metricB(mB), 
                   count(cn) {};

    GlobalMetrics( ContingencyMatrix pR, 
                   CommonMetrics     mR, 
                   unsigned int cn):
                   perfR(pR),   
                   perfG(0,0,0,0),   
                   perfB(0,0,0,0),
                   metricR(mR), 
                   metricG(0,0,0), 
                   metricB(0,0,0), 
                   count(cn) {};

    GlobalMetrics (const GlobalMetrics & rhs) { *this = rhs; };

    bool operator !=(const GlobalMetrics &rhs) const
    {
        return (
                perfR   != rhs.perfR   ||
                perfG   != rhs.perfG   ||
                perfB   != rhs.perfB   ||
                metricR != rhs.metricR ||
                metricG != rhs.metricG ||
                metricB != rhs.metricB ||
                count   != rhs.count);
    };

    //equality operator
    bool operator ==(const GlobalMetrics &rhs) const
    {
        return (
                perfR   == rhs.perfR   &&
                perfG   == rhs.perfG   &&
                perfB   == rhs.perfB   &&
                metricR == rhs.metricR &&
                metricG == rhs.metricG &&
                metricB == rhs.metricB &&
                count   == rhs.count);
    };
    // assigment operator
    GlobalMetrics &operator =(const GlobalMetrics &rhs)
    {
        if (*this != rhs) 
        {
            perfR   = rhs.perfR;
            perfG   = rhs.perfG;
            perfB   = rhs.perfB;
            metricR = rhs.metricR;
            metricG = rhs.metricG;
            metricB = rhs.metricB;
            count   = rhs.count;
        }
        return *this;
    };

    ContingencyMatrix perfR;
    ContingencyMatrix perfG;
    ContingencyMatrix perfB;
    CommonMetrics metricR;
    CommonMetrics metricG;
    CommonMetrics metricB;
    unsigned int count;
};

//typedef struct StatMetrics {
struct StatMetrics {
    StatMetrics () : 
        MeanR(0,0,0),
        MeanG(0,0,0),
        MeanB(0,0,0),
        MedianR(0,0,0),
        MedianG(0,0,0),
        MedianB(0,0,0) {};

    StatMetrics(   CommonMetrics     _meanR, 
                   CommonMetrics     _meanG, 
                   CommonMetrics     _meanB, 
                   CommonMetrics     _medianR, 
                   CommonMetrics     _medianG, 
                   CommonMetrics     _medianB):
                   MeanR(_meanR), 
                   MeanG(_meanG), 
                   MeanB(_meanB), 
                   MedianR(_medianR), 
                   MedianG(_medianG), 
                   MedianB(_medianB)  {};

    StatMetrics (const StatMetrics & rhs) { *this = rhs; };

    bool operator !=(const StatMetrics &rhs) const
    {
        return (
                MeanR   != rhs.MeanR ||
                MeanG   != rhs.MeanG ||
                MeanB   != rhs.MeanB ||
                MedianR != rhs.MedianR ||
                MedianG != rhs.MedianG ||
                MedianB != rhs.MedianB
               );
    };

    bool operator ==(const StatMetrics &rhs) const
    {
        return (
                MeanR   == rhs.MeanR   &&
                MeanG   == rhs.MeanG   &&
                MeanB   == rhs.MeanB   &&
                MedianR == rhs.MedianR &&
                MedianG == rhs.MedianG &&
                MedianB == rhs.MedianB
               );
    };


    StatMetrics &operator =(const StatMetrics &rhs)
    {
        if (*this != rhs) 
        {
            MeanR = rhs.MeanR;
            MeanG = rhs.MeanG;
            MeanB = rhs.MeanB;
            MedianR = rhs.MedianR;
            MedianG = rhs.MedianG;
            MedianB = rhs.MedianB;
        }
        return *this;
    };


    CommonMetrics MeanR;
    CommonMetrics MeanG;
    CommonMetrics MeanB;
    CommonMetrics MedianR;
    CommonMetrics MedianG;
    CommonMetrics MedianB;
};


public:

    //default constructor
    Performance () : 
        FMeasure(0),Variance(0),Mean(0),
        sensitivity(0),specificity(0),precision(0),
        threshold(250),nchannel(1)
    { };

    //Parametric constructor
    Performance (float fm, float var, float mu, unsigned char thr, 
            float sn, float sp, float pr, int nch ) :
        FMeasure(fm),Variance(var),Mean(mu),
        sensitivity(sn),specificity(sp),precision(pr),threshold(thr),nchannel(nch)
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
    string metricsStatisticsAsString() const;
    string rocAsString() const;
    void calculateFinalPerformanceOfMetrics();


    /**
     * Compare both image frames at pixel level
     * Results are saved in TP,TN,FP and FN
     */
    void pixelLevelCompare(const Mat&, const Mat&);
    
    /**
     * Compute number of TP and TN.
     */
    void countPixelsReferenceImage(const Mat&);
    void setThreshold(int th) {threshold = th;};
    //ContingencyMatrix getContingencyMatrix(int idx) {return measure;};

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
   
    double getPSNR(Mat& src1, Mat& src2, int bb=0);
    Scalar getMSSIM( const Mat&, const Mat&);


private:
    /**
     * Obtaind perf indexes of the first channel
     */
    CommonMetrics getPerformance(const ContingencyMatrix&);
    void medianOfMetrics();
    void meanOfMetrics();

    float FMeasure;
    float Variance;
    float Mean;
    float sensitivity;
    float specificity;
    float precision;
    float MCC;
    float medianR[2];
    float medianG;
    float medianB;
    unsigned char threshold;
    unsigned int nchannel;

    static const int MAX_NUMBER_CHANNELS = 3;
    ContingencyMatrix reference;
    ContingencyMatrix current_frame;
    GlobalMetrics accumulated;
    CommonMetrics common_metrics;
    vector<GlobalMetrics> vectorMetrics;
    StatMetrics stat;
};


}
#endif
