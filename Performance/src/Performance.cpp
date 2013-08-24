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

#include "Performance.h"
#include <math.h>

#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision

using namespace cv;
using namespace std;

namespace bgs {

const float Performance::PEAK_PARAMETER = (float)5/2;
//const float Performance::PEAK_PARAMETER = (float)1;
    
    
    
/*
 * Return as string a summary of TP and TN reference
 * and TP TN FP FN sensitivity specificity and MCC 
 * obtained of comparing reference and mask.
 */
string Performance::asString() const
{
    stringstream str;

    if (pixel_performance) {
        
        str 
        << this->refToString() << "    "
        << current_frame.tp    << " " 
        << current_frame.tn    << " " 
        << current_frame.fp    << " " 
        << current_frame.fn    << "    "
        << std::scientific     << sensitivity     << " " 
        << std::scientific     << (1-specificity) << " "
        << std::scientific     << FMeasure        << " " 
        << std::scientific     << MCC             << "    ";
        
    }
    if (frame_performance) {
        
        str 
        << setiosflags(ios::fixed) << setprecision(2) << similarity_frame.PSNR << " "
        << setiosflags(ios::fixed) << setprecision(2) << similarity_frame.MSSIM << " "
        << setiosflags(ios::fixed) << setprecision(2) << similarity_frame.DSCORE;
        
    }
    
    return str.str();
}

/*
 * Return a string TP and TN of reference
 */
string Performance::refToString() const
{
    stringstream str;
    str << reference.tp << " " << reference.tn;
    return str.str();

}

/*
 * Returns total accumulated of
 * TP TN FP FN SENSITIVITY SPECIFICITY and MCC
 */
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

/*
 * Returns mean accumulated of
 * TP TN FP FN SENSITIVITY SPECIFICITY and MCC
 */
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
    
/*
 * Returns a summary of MEAN and MEDIAN values of total accumulated
 * TP TN FP FN SENSITIVITY SPECIFICITY and MCC
 */
string Performance::metricsStatisticsAsString() const
{
    double meanFPR   = 1-(double)stat.MeanR.specificity; 
    double medianFPR = 1-(double)stat.MedianR.specificity;

    stringstream str;
    if (pixel_performance) {

        str 
        << std::scientific << stat.MeanR.sensitivity    << " " 
        << std::scientific << meanFPR                   << " "
        << std::scientific << stat.MeanR.f1score        << " "
        << std::scientific << stat.MeanR.MCC            << "    "
        << std::scientific << stat.MedianR.sensitivity  << " " 
        << std::scientific << medianFPR                 << " "
        << std::scientific << stat.MedianR.f1score      << " "
        << std::scientific << stat.MedianR.MCC          << "    ";
    
    }
    if (frame_performance) {

        str 
        << std::scientific << similarity_mean.PSNR      << " "
        << std::scientific << similarity_mean.MSSIM     << " "
        << std::scientific << similarity_mean.DSCORE    << "    "
        << std::scientific << similarity_median.PSNR    << " "
        << std::scientific << similarity_median.MSSIM   << " "
        << std::scientific << similarity_median.DSCORE  ;
    }
    
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

string Performance::
getHeaderForFileWithNameOfStatisticParameters()
{
    stringstream str;
    
    if (pixel_performance) {
        str << "TPR_MEAN FPR_MEAN FMEASURE_MEAN MCC_MEAN  TPR_MEDIAN FPR_MEDIAN FMEASURE_MEDIAN MCC_MEDIAN    ";
    }
    if (frame_performance) {
        str << "PSNR MSSIM DSCORE";
    }
    
    return str.str();
        
}

// Calculate performance metrics based in pixel level compararation
// Return results in CommonMetric struct.
Performance::CommonMetrics 
Performance::getPerformance(const ContingencyMatrix& mt)
{
    if (mt == ContingencyMatrix(0,0,0,0) ) {
        sensitivity = 0;
        specificity = 0;
        FMeasure    = 0;
        MCC         = 0;
        return CommonMetrics(0,0,0,0);
    }

    double TP = (double)mt.tp;
    double TN = (double)mt.tn;
    double FP = (double)mt.fp;
    double FN = (double)mt.fn;

    sensitivity = TP/(TP + FN);// it is also Recall
    specificity = TN/(TN + FP);
    precision   = TP/(TP + FP);

    //MCC = ((TP*TN)–(FP*FN))/sqrt((TP + FP)*(TP + TN)*(FP + FN)*(TN + FN));
    double numerator = ((TP*TN)-(FP*FN));
    //double denominator= sqrt((TP+FP)*(TP+TN)*(FP+FN)*(TN+FN));
    double denominator= sqrt((TP+FP)*(TP+FN)*(TN+FP)*(TN+FN));
    if (denominator == 0)
        denominator = 1;
    MCC = numerator / denominator;
    
    // Calculation of FMeasure
    numerator = 2 * precision * sensitivity;
    denominator = precision + sensitivity;
    if (denominator == 0)
        FMeasure = 0;
    else {
        FMeasure = numerator/denominator;
    }


    return CommonMetrics(sensitivity,specificity,FMeasure,MCC);
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
        cvtColor( _image, _img, CV_BGR2GRAY );
    
    Mat _ref = _reference;
    if (_reference.channels() > 1) 
        cvtColor( _reference, _ref, CV_BGR2GRAY );
    
    
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
    
    // Check out TP TN FP and FN
    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        const uchar* _ref_data= _ref.ptr<uchar>(j);
        const uchar* _img_data= _img.ptr<uchar>(j);
        
        for (int i=0; i<nc; i++) {
            //convert images to 0 or 1, binary form
            // 1: white 0: black
            uchar uref = _ref_data[i] > 0 ? 1 : 0;
            uchar ucmp = _img_data[i] > 128 ? 1 : 0;
            
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
    
    
    //calculates sensitivity (TPR) and specificity (1-FPR)
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
                                          CommonMetrics(0,0,0,0), 
                                          CommonMetrics(0,0,0,0), 
                                          ptrGlobal->count));
    
    
    //duration = static_cast<double>(cv::getTickCount())-duration;
    //duration /= cv::getTickFrequency(); //the elapsed time in ms
    //cout << "Duration pixelLevelCompare : " << duration << endl; 
}

    


// Counts up from reference frame number of TP and TN
void Performance::countPixelsReferenceImage(const Mat& image) 
{
    //double duration;
    //duration = static_cast<double>(cv::getTickCount());

    
    // get Mat header for input image. This is O(1) operation
    Mat img = image;

    // Check and convert reference image to gray
    if (image.channels() > 1) 
        cvtColor( image, img, CV_BGR2GRAY );


    reference = ContingencyMatrix();
    ContingencyMatrix *r_ptr = &reference;

    r_ptr->tp = countNonZero(img);
    r_ptr->tn = img.rows*img.cols - r_ptr->tp;
    
    
    //duration = static_cast<double>(cv::getTickCount())-duration;
    //duration /= cv::getTickFrequency(); //the elapsed time in ms
    //cout << "BENCHMARK DURATION : " << duration << endl;



}


    
/*    
// Counts up from reference frame number of TP and TN
void Performance::countPixelsReferenceImage(const Mat& image) 
{
 double duration;
 duration = static_cast<double>(cv::getTickCount());

    
    // get Mat header for input image. This is O(1) operation
    Mat img = image;
    
    // Check and convert reference image to gray
    if (image.channels() > 1) 
        cvtColor( image, img, CV_BGR2GRAY );
    
    
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
            
            //if (data[i] > threshold)
            if (data[i] > 0)
                r_ptr->tp++;
            else
                r_ptr->tn++;
        }
    }
    
 
 duration = static_cast<double>(cv::getTickCount())-duration;
 duration /= cv::getTickFrequency(); //the elapsed time in ms
 cout << "BENCHMARK DURATION : " << duration << endl;

    
    
}
*/
    

    
void Performance::frameSimilarity(InputArray _truth,InputArray _mask)
{
    
    //double duration;
    //duration = static_cast<double>(cv::getTickCount());
    
    // Verify consistency of both frames
    if ( _truth.size() != _mask.size() ) {
        cerr << "Invalid size :  " << _truth.size() << " " << _mask.size() << endl;
        return;
    }
    if ( _truth.type() != _mask.type() ) {
        cerr << "Invalid type :  " 
        << _truth.type() << " " << _truth.depth() << " " << _truth.channels() <<  "  "
        << _mask.type()  << " " << _mask.depth()  << " " << _mask.channels()  <<  endl;
        return;
    }
    

    // Convert both frames to single channel
    // Check and convert foreground mask image to gray
    Mat TRUTH;
    Mat FOREGROUND;
    if (_truth.channels() > 1) 
        cvtColor( _truth.getMat(), TRUTH, CV_BGR2GRAY );
    else 
        TRUTH = _truth.getMat();

    if (_mask.channels() > 1) 
        cvtColor( _mask.getMat(), FOREGROUND, CV_BGR2GRAY );
    else 
        FOREGROUND = _mask.getMat();
    
    //Initialize to zero all internal values of the struct.
    similarity_frame = Similarity();
    Similarity *m_ptr = &similarity_frame;


    Mat MAP;
    m_ptr->PSNR   = getPSNR(TRUTH, FOREGROUND);
    m_ptr->MSSIM  = getMSSIM(TRUTH, FOREGROUND);
    m_ptr->DSCORE = getDScore(FOREGROUND, TRUTH, MAP);

    
    
    similarity_accumulated += similarity_frame;
    vectorSimilarity.push_back(similarity_frame);


    
}
  
    
void Performance::frameSimilarity(InputArray _mask, InputArray _truth, InputArray map)
{
    
    //double duration;
    //duration = static_cast<double>(cv::getTickCount());

    // Verify if mapt struct was previously filled
    // In case an empty map compute general map. 
    Mat EMAP = map.getMat();    

    
    
    // Verify consistency of both frames
    if ( _truth.size() != _mask.size() ) {
        cerr << "Invalid size :  " << _truth.size() << " " << _mask.size() << endl;
        return;
    }
    if ( _truth.type() != _mask.type() ) {
        cerr << "Invalid type :  " 
        << _truth.type() << " " << _truth.depth() << " " << _truth.channels() <<  "  "
        << _mask.type()  << " " << _mask.depth()  << " " << _mask.channels()  <<  endl;
        return;
    }
    
    
    // Convert both frames to single channel
    // Check and convert foreground mask image to gray
    Mat TRUTH;
    Mat FOREGROUND;
    if (_truth.channels() > 1) 
        cvtColor( _truth.getMat(), TRUTH, CV_BGR2GRAY );
    else 
        TRUTH = _truth.getMat();
    
    if (_mask.channels() > 1) 
        cvtColor( _mask.getMat(), FOREGROUND, CV_BGR2GRAY );
    else 
        FOREGROUND = _mask.getMat();
    
    //Initialize to zero all internal values of the struct.
    similarity_frame = Similarity();
    Similarity *m_ptr = &similarity_frame;
    
    
    
    m_ptr->PSNR   = getPSNR(TRUTH, FOREGROUND);
    m_ptr->MSSIM  = getMSSIM(TRUTH, FOREGROUND);
    m_ptr->DSCORE = getDScore(FOREGROUND, TRUTH, EMAP);
    
    
    
    similarity_accumulated += similarity_frame;
    vectorSimilarity.push_back(similarity_frame);
    
    
    
}
    
    
    

void Performance::meanOfMetrics() 
{
    if (accumulated.count == 0)
        return;

    if ( pixel_performance ) {
        GlobalMetrics *g_p = &accumulated;
        unsigned int   cnt = g_p->count;

        stat.MeanR = CommonMetrics( g_p->metricR.sensitivity/cnt, 
                                    g_p->metricR.specificity/cnt,
                                    g_p->metricR.f1score/cnt,
                                    g_p->metricR.MCC/cnt);
        stat.MeanG = CommonMetrics( 0,0,0,0 );
        stat.MeanB = CommonMetrics( 0,0,0,0 );
    
    }
    
    
    // Just for now calculate similarity mean in this way,
    // in a second revision, I'll put all those values
    // in a common structure
    if (frame_performance) {
 
        unsigned int size = vectorSimilarity.size();
        if (size > 0) {
            Similarity *p_sim = &similarity_accumulated;
            similarity_mean = Similarity((double)p_sim->PSNR/(double)size,
                                         (double)p_sim->MSSIM/(double)size,
                                         (double)p_sim->DSCORE/(double)size);
        }
    }
}

void Performance::medianOfMetrics() 
{
    vector<double> sen;
    vector<double> spe;
    vector<double> f1m;
    vector<double> mcc;
    
    if ( pixel_performance ) {
        unsigned int i = (unsigned int)vectorMetrics.size()/2; 
        bool even      = vectorMetrics.size() % 2 ? false: true;

        for(vector<GlobalMetrics>::iterator it = vectorMetrics.begin(); 
                                            it != vectorMetrics.end(); ++it) 
        {
            sen.push_back(it->metricR.sensitivity);
            spe.push_back(it->metricR.specificity);
            f1m.push_back(it->metricR.f1score);
            mcc.push_back(it->metricR.MCC);
        }

        sort (sen.begin(), sen.end());
        sort (spe.begin(), spe.end());
        sort (f1m.begin(), f1m.end());
        sort (mcc.begin(), mcc.end());


        if (even) {
            stat.MedianR = CommonMetrics( (sen[i]+sen[i-1])/2,
                                          (spe[i]+spe[i-1])/2,
                                          (f1m[1]+f1m[i-1])/2,
                                          (mcc[i]+mcc[i-1])/2);
        } else {
            stat.MedianR = CommonMetrics( sen[i],spe[i],f1m[i],mcc[i]);
        }
    
    }
    /*
    // Getting median of similarity
    unsigned int size = (unsigned int)vectorSimilarity.size()/2; 
    bool size_even    = vectorSimilarity.size() % 2 ? false: true;
    
    vector<double> _psnr;
    vector<double> _mssim;
    vector<double> _dscore;
    
    for (vector<Similarity>::iterator it = vectorSimilarity.begin(); it != vectorSimilarity.end(); ++it) {
        _psnr.push_back(it->PSNR);
        _mssim.push_back(it->MSSIM);
        _dscore.push_back(it->DSCORE);
    }
    
    sort(_psnr.begin()  , _psnr.end());
    sort(_mssim.begin() , _mssim.end());
    sort(_dscore.begin(), _dscore.end());
    
    if (size_even)
        similarity_median = Similarity( (_psnr  [size] + _psnr  [size-1])/2,
                                        (_mssim [size] + _mssim [size-1])/2,
                                        (_dscore[size] + _dscore[size-1])/2 );
    else
        similarity_median = Similarity( _psnr[size], _mssim[size], _dscore[size] );
    */                                   


}
    
    
/*
 * Performance calculation of accumulated metrics
 */
void Performance::calculateFinalPerformanceOfMetrics()
{
    this->medianOfMetrics();
    this->meanOfMetrics();
}


/*
 * PSNR Measurement taken from Opencv 
 */
double Performance::getPSNR(const Mat& I1, const Mat& I2)
{
    Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2
    
    Scalar s = sum(s1);        // sum elements per channel
    
    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels
    
    if( sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double mse  = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}


    
/*
double Performance::getPSNR(Mat& src1, Mat& src2, int bb)
{
    double duration;
    duration = static_cast<double>(cv::getTickCount());

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

    duration = static_cast<double>(cv::getTickCount())-duration;
    duration /= cv::getTickFrequency(); //the elapsed time in ms
    
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
*/
    
    

    
    
/*
double getPSNR(const Mat& I1, const Mat& I2)
{
    double duration;
    duration = static_cast<double>(cv::getTickCount());

    Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);        // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    duration = static_cast<double>(cv::getTickCount())-duration;
    duration /= cv::getTickFrequency(); //the elapsed time in ms

    if( sse <= 1e-10) // for small values return zero
                return 0;
    else
    {
        double mse  = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}
*/
    
    
    
double Performance::getMSSIM( const Mat& i1, const Mat& i2)
{
    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/
    int d = CV_32F;

    Mat I1, I2;
    i1.convertTo(I1, d);            // cannot calculate on one byte large values
    i2.convertTo(I2, d);

    Mat I2_2   = I2.mul(I2);        // I2^2
    Mat I1_2   = I1.mul(I1);        // I1^2
    Mat I1_I2  = I1.mul(I2);        // I1 * I2

    /*************************** END INITS **********************************/

    Mat mu1, mu2;                   // PRELIMINARY COMPUTING
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);

    Mat mu1_2   =   mu1.mul(mu1);
    Mat mu2_2   =   mu2.mul(mu2);
    Mat mu1_mu2 =   mu1.mul(mu2);

    Mat sigma1_2, sigma2_2, sigma12;

    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    ///////////////////////////////// FORMULA ////////////////////////////////

    Mat t1, t2, t3;

    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);                 // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);                 // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

    Mat ssim_map;
    divide(t3, t1, ssim_map);        // ssim_map =  t3./t1;

    Scalar mssim = mean(ssim_map);   // mssim = average of ssim map
    return mssim.val[0];
}



    
/**
 * Foreground map is already computed
 */
double Performance::getDScore(InputArray foreground, InputArray groundtruth, InputArray map)
{
    Mat Mask;
    Mat Mask_BINARY;
    Mat Mask_BINARY_INV;
    Mat Truth;
    Mat Truth_BINARY_INV;
    Mat Truth_BINARY;
    Mat EMAP;

    
    //double duration;
    //duration = static_cast<double>(cv::getTickCount());

    
    // Check and convert foreground mask image to gray
    if (foreground.channels() > 1) 
        cvtColor( foreground.getMat(), Mask, CV_BGR2GRAY );
    else 
        Mask = foreground.getMat();
    
    
    if (groundtruth.channels() > 1) 
        cvtColor( groundtruth.getMat(), Truth, CV_BGR2GRAY );
    else 
        Truth = groundtruth.getMat();
    
    
    if (map.empty())
        computeGeneralDSCoreMap(Truth, EMAP);
    else {
        EMAP = map.getMat();
    }
    
    
    Mask.convertTo(Mask, CV_32F);
    Truth.convertTo(Truth, CV_32F);


    // binary ground truth 
    cv::threshold(Truth, Truth_BINARY    , 1, 1, THRESH_BINARY);
    cv::threshold(Truth, Truth_BINARY_INV, 1, 1, THRESH_BINARY_INV);
    cv::threshold(Mask, Mask_BINARY    , 1, 1, THRESH_BINARY);
    cv::threshold(Mask, Mask_BINARY_INV, 1, 1, THRESH_BINARY_INV);

    Mat FOREGROUND;
    Mat BACKGROUND;
    
    bitwise_and(Truth_BINARY, Mask_BINARY_INV, FOREGROUND);
    bitwise_and(Truth_BINARY_INV, Mask_BINARY, BACKGROUND);
    
    
    
    Mat FRAME_MAP;
    bitwise_or(FOREGROUND, BACKGROUND, FRAME_MAP);
    

    
    FRAME_MAP = FRAME_MAP.mul(EMAP);
    

    
    Scalar DScore = mean(FRAME_MAP);
    
    
    //normalize(FRAME_MAP, FRAME_MAP, 0, 255, cv::NORM_MINMAX);
    //imwrite("frame_map.png", FRAME_MAP);
    
    
    //duration = static_cast<double>(cv::getTickCount())-duration;
    //duration /= cv::getTickFrequency(); //the elapsed time in ms
    //cout << "getDScore Duration: " << duration << " " << DScore.val[0] << endl; 

    
    
    return DScore.val[0];
    
}    
    
    
    
    
void Performance::computeGeneralDSCoreMap(InputArray img, OutputArray map)
{
    // Check and convert foreground mask image to gray
    Mat MASK;
    if (img.channels() > 1) 
        cvtColor( img.getMat(), MASK, CV_BGR2GRAY );
    else 
        MASK = img.getMat();

    Mat MASK_BINARY;
    Mat MASK_BINARY_INV;
    
    // Convert image to binary
    cv::threshold(MASK, MASK_BINARY    , 1, 1, THRESH_BINARY);
    cv::threshold(MASK, MASK_BINARY_INV, 1, 1, THRESH_BINARY_INV);
    
    Mat MAP_FOREGROUND;
    Mat MAP_BACKGROUND;
        
    // Get distance of foreground and background.
    distanceTransform(MASK_BINARY    , MAP_FOREGROUND, CV_DIST_L1, 3);
    distanceTransform(MASK_BINARY_INV, MAP_BACKGROUND, CV_DIST_L1, 3);
    
    MAP_FOREGROUND *=5;

    Mat EMAP = MAP_FOREGROUND + MAP_BACKGROUND;
    EMAP *=2; 
    
    Mat LOG_EMAP;
    log(EMAP, LOG_EMAP);
    LOG_EMAP -= PEAK_PARAMETER;
    Mat LOG_EMAP_2 = LOG_EMAP.mul(LOG_EMAP);
    LOG_EMAP_2 *= -1;
    Mat EXP_LOG_EMAP_2;
    //    
    exp(LOG_EMAP_2, EXP_LOG_EMAP_2);
    
    EXP_LOG_EMAP_2.copyTo(map);

        
        
}
    
    
 
    
    
    

    
    
    
}
