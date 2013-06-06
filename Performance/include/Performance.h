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
typedef struct Indexes
{
    Indexes (): tp(0),tn(0),fp(0),fn(0) {};
    Indexes(float _tp, float _tn, float _fp, float _fn):
        tp(_tp),tn(_tn),fp(_fp),fn(_fn) {};
    //copy constructor
    Indexes (const Indexes & rhs) { *this = rhs; };
    //Inequality operator
    bool operator !=(const Indexes &rhs) const
    {
        return ((tp!=rhs.tp)||(tn!=rhs.tn)||(fp!=rhs.fp)||(fn!=rhs.fn));
    };
    //equality operator
    bool operator ==(const Indexes &rhs) const
    {
        return ((tp==rhs.tp)&&(tn==rhs.tn)&&(fp==rhs.fp)&&(fn==rhs.fn));
    };
    // assigment operator
    Indexes &operator =(const Indexes &rhs)
    {
        if (*this != rhs) 
        {
            tp = rhs.tp; tn = rhs.tn; fp = rhs.fp; fn = rhs.fn;
        }
        return *this;
    };
    // overloaded += operator
    Indexes& operator +=(const Indexes &rhs)
    {
       tp += rhs.tp; tn += rhs.tn; fp += rhs.fp; fn += rhs.fn;
       return *this;
    };

    const Indexes operator+(const Indexes &rhs) const
    {
       //Indexes result(rhs.tp,rhs.tn,rhs.fp,rhs.fn);
       Indexes result(*this);
       return result += rhs;
    }
/*
    const Indexes operator+(const Indexes &lhs,const Indexes &rhs) const
    {
       Indexes result(rhs.tp + lhs.tp,
                      rhs.tn + lhs.tn,
                      rhs.fp + lhs.fp,
                      rhs.fn + lhs.fn);
       return result;
    }

    const Indexes operator-(const Indexes &lhs,const Indexes &rhs)
    {
       Indexes result(rhs.tp - lhs.tp,
                      rhs.tn - lhs.tn,
                      rhs.fp - lhs.fp,
                      rhs.fn - lhs.fn);
       return result;
    }
*/
    // overloaded -= operator
    Indexes& operator -=(const Indexes &rhs) 
    {
        tp -= rhs.tp; tn -= rhs.tn; fp -= rhs.fp; fn -= rhs.fn;
        return *this;
    };

    float tp;
    float tn;
    float fp;
    float fn;
};

public:

    //default constructor
    Performance () : FMeasure(0),Variance(0),Mean(0),threshold(250)
    { };

    //Parametric constructor
    Performance (float fm, float var, float mu, unsigned char thr ) :
        FMeasure(fm),Variance(var),Mean(mu),threshold(thr)
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
        }
        return *this;
    };

    //Equality operator
    bool operator ==(const Performance &rhs) const
    {
        return ((FMeasure      == rhs.FMeasure)      &&
                (Variance      == rhs.Variance)      &&
                (Mean          == rhs.Mean));
    };

    //Inequality operator
    bool operator !=(const Performance &rhs) const
    {
        return ((FMeasure      != rhs.FMeasure)      ||
                (Variance      != rhs.Variance)      ||
                (Mean          != rhs.Mean));
    };

    // overloaded += operator
    Performance& operator +=(const Performance &rhs)
    {
        FMeasure      += rhs.FMeasure;
        Variance      += rhs.Variance;
        Mean          += rhs.Mean;
        return *this;
    };

    const Performance operator+(const Performance &rhs) const
    {
        Performance result(rhs.FMeasure,rhs.Variance,rhs.Mean,rhs.threshold);
        return result += rhs;
    }

    // overloaded -= operator
    Performance& operator -=(const Performance &rhs) 
    {
        FMeasure      -= rhs.FMeasure;
        Variance      -= rhs.Variance;
        Mean          -= rhs.Mean;
        return *this;
    };

    //Return an string with numbe of the values
    string asString() const;
    string refToString() const;

    inline float getSensitivity() { return sensitivity;};
    inline float getSpecificity() { return specificity;};
    float precision();

    /**
     * Compare both image frames at pixel level
     * Results are saved in TP,TN,FP and FN
     */
    void pixelLevelCompare(const Mat&, const Mat&);
    
    /**
     * Compare both frames 
     * TP, TN, FP and FN are calculated.
     */
    void compareFrames(const Mat&, const Mat&);

    /**
     * Compute number of TP and TN.
     */
    void setPixelsReference(const Mat&);
    void setThreshold(int th) {threshold = th;};
    //void binaryCountNumberPixelsReferenceFrame(const Mat&);
    float recall() {return sensitivity;};
    float TPR() {return sensitivity; };
    float TNR() {return (1 - specificity);};  

    inline float FM() {return FMeasure;};
    inline float Sigma() {return Variance; };
    inline float Mu()  { return Mean;};
    Indexes getIndexes(int idx) {return measure[idx];};
private:

    Indexes ref[3];
    Indexes measure[3];
    float FMeasure;
    float Variance;
    float Mean;
    float specificity;
    float sensitivity;
    unsigned char threshold;
};


}
#endif
