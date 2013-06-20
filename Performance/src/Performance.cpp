/*******************************************************************************
 * <Self-Adaptive Gaussian Mixture Model.>
 * Copyright (C) <2013>  <name of author>
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

#include "Performance.h"
#include <math.h>

using namespace cv;
using namespace std;

namespace bgs {


string Performance::asString() const
{
    stringstream str;  
    str << this->refToString() << "    "
        << current_frame.tp    << " " 
        << current_frame.tn    << " " 
        << current_frame.fp    << " " 
        << current_frame.fn    << "    "
        << std::scientific     << sensitivity << " " 
        << std::scientific     << specificity << " "
        << std::scientific     << MCC;
    return str.str();
}

string Performance::refToString() const
{
    stringstream str;
    str << reference.tp << " " << reference.tn;
    return str.str();

}

string Performance::summaryAsString() const
{
    stringstream str;

    const GlobalMetrics *acc = &accumulated;
    unsigned int i = acc->count; 
    str << i << " 0 0     " 
        << acc->perfR.tp << " " 
        << acc->perfR.tn << " " 
        << acc->perfR.fp << " " 
        << acc->perfR.fn << "    "
        << std::scientific << acc->metricR.sensitivity << " " 
        << std::scientific << acc->metricR.specificity << " "
        << std::scientific << acc->metricR.MCC;
    return str.str();

}



string Performance::averageSummaryAsString() const
{
    stringstream str;

    const GlobalMetrics *acc = &accumulated;
    unsigned int i = acc->count; 
    str << i << " 0 0    " 
        << acc->perfR.tp/i << " " 
        << acc->perfR.tn/i << " " 
        << acc->perfR.fp/i << " " 
        << acc->perfR.fn/i << "    "
        << std::scientific << acc->metricR.sensitivity/i << " " 
        << std::scientific << acc->metricR.specificity/i << " "
        << std::scientific << acc->metricR.MCC/i ;
    return str.str();

}

string Performance::metricsStatisticsAsString() const
{
    double mean   = 1-(double)stat.MeanR.specificity; 
    double median = 1-(double)stat.MedianR.specificity;

    stringstream str;
    str << std::scientific << stat.MeanR.sensitivity    << " " 
        << std::scientific << mean                      << " "
        << std::scientific << stat.MeanR.specificity    << " "
        << std::scientific << stat.MeanR.MCC            << "    "
        << std::scientific << stat.MedianR.sensitivity  << " " 
        << std::scientific << median                    << " "
        << std::scientific << stat.MedianR.specificity  << " "
        << std::scientific << stat.MedianR.MCC;
    return str.str();
}

string Performance::rocAsString() const
{
    double fpr1   = 1-(double)stat.MeanR.specificity; 
    double fpr2   = 1-(double)stat.MedianR.specificity;

    stringstream str;
    str << std::scientific << stat.MeanR.sensitivity    << " " 
        << std::scientific << fpr1                      << " "
        << std::scientific << stat.MeanR.MCC            << "    "
        << std::scientific << stat.MedianR.sensitivity  << " " 
        << std::scientific << fpr2                      << " "
        << std::scientific << stat.MedianR.MCC ;
    return str.str();
}


// Calculate TPR and FPR 
Performance::CommonMetrics 
Performance::getPerformance(const ContingencyMatrix& mt)
{
    if (mt == ContingencyMatrix(0,0,0,0) ) {
        sensitivity = 0;
        specificity = 0;
        MCC         = 0;
        return CommonMetrics(0,0,0);
    }

    double TP = (double)mt.tp;
    double TN = (double)mt.tn;
    double FP = (double)mt.fp;
    double FN = (double)mt.fn;

    sensitivity = TP/(TP + FN);
    specificity = TN/(TN + FP);
    precision   = TP/(TP + FP);

    //MCC = ((TP*TN)–(FP*FN))/sqrt((TP + FP)*(TP + TN)*(FP + FN)*(TN + FN));
    double numerator = ((TP*TN)-(FP*FN));
    double denominator= sqrt((TP+FP)*(TP+TN)*(FP+FN)*(TN+FN));
    MCC = numerator / denominator;


    return CommonMetrics(sensitivity,specificity,MCC);
}



void Performance::pixelLevelCompare(const Mat& _reference, const Mat& _image)
{
    //double duration;
    //duration = static_cast<double>(cv::getTickCount());

    if ( _reference.dims > 2 || _image.dims > 2 ) {
        cerr << "Invalid dimensions :  " << _reference.dims << " " << _image.dims << endl;
        return;
    }

    if ( _reference.size() != _image.size() ) {
        cerr << "Invalid size :  " << _reference.size() << " " << _image.size() << endl;
        return;
    }
    if ( _reference.type() != _image.type() ) {
        cerr << "Invalid type :  " 
            << _reference.type() << " " << _reference.depth() << " " << _reference.channels() <<  "  "
            << _image.type() << " " << _image.depth() << " " << _image.channels() <<  endl;
        return;
    }
    
    // get Mat header for input image. This is O(1) operation
    Mat _img = _image;
    // Check and convert reference image to gray
    if (_image.channels() > 1) 
        cvtColor( _image, _img, CV_RGB2GRAY );

    Mat _ref = _reference;
    if (_reference.channels() > 1) 
        cvtColor( _reference, _ref, CV_RGB2GRAY );


    //Converted to gray in case of color image and
    //counts TP and TN in pixelsRefImage variable
    this->countPixelsReferenceImage(_ref);
    
    // number of lines
    int nl= _ref.rows; 
    int nc= _ref.cols;

    if ( _ref.isContinuous() && _img.isContinuous() )
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }
   
    //Initialize to zero all internal values of the struct.
    current_frame = ContingencyMatrix();
    ContingencyMatrix *m_ptr = &current_frame;

    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        const uchar* _ref_data= _ref.ptr<uchar>(j);
        const uchar* _img_data= _img.ptr<uchar>(j);
        
        for (int i=0; i<nc; i++) {
            
            //convert images to 0 or 1, binary form
            // 1: white 0: black
            uchar uref = _ref_data[i]/200;
            uchar ucmp = _img_data[i]/200;
            
            if (uref == ucmp) {
                if (uref)//silhouette
                    m_ptr->tp++;
                else
                    //background
                    m_ptr->tn++;
            }
            else {
                if (uref)
                    m_ptr->fn++;
                else
                    m_ptr->fp++;
            }
        }
    }
    

    //calculate sensitivity (TPR) and specificity (1-FPR)
    CommonMetrics img_metrics = getPerformance(current_frame);

    //save an accumulate value of TP,TN...
    GlobalMetrics *ptrGlobal = &accumulated;
    ptrGlobal->perfR   += current_frame;
    ptrGlobal->metricR += img_metrics;
    ptrGlobal->count += 1;

    // Save each current_frame in a global vector
    vectorMetrics.push_back(
            GlobalMetrics(current_frame, 
                ContingencyMatrix(0,0,0,0), 
                ContingencyMatrix(0,0,0,0), 
                img_metrics, 
                CommonMetrics(0,0,0), 
                CommonMetrics(0,0,0), 
                ptrGlobal->count));


    //duration = static_cast<double>(cv::getTickCount())-duration;
    //duration /= cv::getTickFrequency(); //the elapsed time in ms
}





// Counts up from reference frame number of TP and TN
void Performance::countPixelsReferenceImage(const Mat& image) 
{
    // get Mat header for input image. This is O(1) operation
    Mat img = image;

    // Check and convert reference image to gray
    if (image.channels() > 1) 
        cvtColor( image, img, CV_RGB2GRAY );


    reference = ContingencyMatrix();
    ContingencyMatrix *r_ptr = &reference;

    int nl= img.rows; // number of lines
    int nc= img.cols;

    if (img.isContinuous())
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }

    //this loop is executed only once
    // in case of continuous images
    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        const uchar* data= img.ptr<uchar>(j);
        for (int i=0; i<nc; i++) {

                if (data[i] > threshold)
                    r_ptr->tp++;
                else
                    r_ptr->tn++;
        }
    }

}

void Performance::meanOfMetrics() 
{
    if (accumulated.count == 0)
        return;

    GlobalMetrics *g_p = &accumulated;
    unsigned int cnt = g_p->count;

    stat.MeanR = CommonMetrics( g_p->metricR.sensitivity/cnt, 
                                g_p->metricR.specificity/cnt,
                                g_p->metricR.MCC/cnt);
    stat.MeanG = CommonMetrics( 0,0,0 );
    stat.MeanB = CommonMetrics( 0,0,0 );

}

void Performance::medianOfMetrics() 
{
    vector<double> sen;
    vector<double> spe;
    vector<double> mcc;
    
    unsigned int i = (unsigned int)vectorMetrics.size()/2; 
    bool even      = vectorMetrics.size() % 2 ? false: true;

    for(vector<GlobalMetrics>::iterator it = vectorMetrics.begin(); 
                                        it != vectorMetrics.end(); ++it) 
    {
        sen.push_back(it->metricR.sensitivity);
        spe.push_back(it->metricR.specificity);
        mcc.push_back(it->metricR.MCC);
    }

    sort (sen.begin(), sen.end());
    sort (spe.begin(), spe.end());
    sort (mcc.begin(), mcc.end());


    if (even) {
        stat.MedianR = CommonMetrics( (sen[i]+sen[i+1])/2,
                                      (spe[i]+spe[i+1])/2,
                                      (mcc[i]+mcc[i+1])/2);
    } else {
        stat.MedianR = CommonMetrics( sen[i+1],spe[i+1],mcc[i+1]);
    }

}

void Performance::calculateFinalPerformanceOfMetrics()
{
    this->medianOfMetrics();
    this->meanOfMetrics();
}

}


double getPSNR(Mat& src1, Mat& src2, int bb)
{
    int i,j;
    double sse,mse,psnr;
    sse = 0.0;

    Mat s1,s2;
    cvtColor(src1,s1,CV_BGR2GRAY);
    cvtColor(src2,s2,CV_BGR2GRAY);

    int count=0;
    for(j=bb;j<s1.rows-bb;j++)
    {
        uchar* d=s1.ptr(j);
        uchar* s=s2.ptr(j);

        for(i=bb;i<s1.cols-bb;i++)
        {
            sse += ((d[i] - s[i])*(d[i] - s[i]));
            count++;
        }
    }
    if(sse == 0.0 || count==0)
    {
        return 0;
    }
    else
    {
        mse =sse /(double)(count);
        psnr = 10.0*log10((255*255)/mse);
        return psnr;
    }
}

