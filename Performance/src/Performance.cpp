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

using namespace cv;
using namespace std;

namespace bgs {


string Performance::asString() const
{
    stringstream str;  
    str << this->refToString() << " "
        << measure[0].tp << " " << measure[0].tn << " " << measure[0].fp << " " << measure[0].fn << " "
        << measure[1].tp << " " << measure[1].tn << " " << measure[1].fp << " " << measure[1].fn << " "
        << measure[2].tp << " " << measure[2].tn << " " << measure[2].fp << " " << measure[2].fn << " "
        << sensitivity << " " << specificity;
    return str.str();
}

string Performance::refToString() const
{
    stringstream str;
    str << refImageArray[0].tp << " " << refImageArray[0].tn << " " 
        << refImageArray[1].tp << " " << refImageArray[1].tn << " "
        << refImageArray[2].tp << " " << refImageArray[2].tn;
    return str.str();

}

string Performance::summaryAsString() const
{
    stringstream str;

    const GlobalMetrics *ptrGlobal = &global_metrics;
    unsigned int i = ptrGlobal->count; 
    str << i << " 0 0 0 0 0 0 " 
        << ptrGlobal->perfR.tp << " " << ptrGlobal->perfR.tn << " " << ptrGlobal->perfR.fp << " " << ptrGlobal->perfR.fn << " "
        << ptrGlobal->perfG.tp << " " << ptrGlobal->perfG.tn << " " << ptrGlobal->perfG.fp << " " << ptrGlobal->perfG.fn << " "
        << ptrGlobal->perfB.tp << " " << ptrGlobal->perfB.tn << " " << ptrGlobal->perfB.fp << " " << ptrGlobal->perfB.fn << " "
        << ptrGlobal->metricR.sensitivity << " " << ptrGlobal->metricR.specificity << " " 
        << ptrGlobal->metricG.sensitivity << " " << ptrGlobal->metricG.specificity << " " 
        << ptrGlobal->metricB.sensitivity << " " << ptrGlobal->metricB.specificity ;
    return str.str();

}



string Performance::averageSummaryAsString() const
{
    stringstream str;

    const GlobalMetrics *ptrGlobal = &global_metrics;
    unsigned int i = ptrGlobal->count; 
    str << i << " 0 0 0 0 0 0 " 
        << ptrGlobal->perfR.tp/i << " " << ptrGlobal->perfR.tn/i << " " << ptrGlobal->perfR.fp/i << " " << ptrGlobal->perfR.fn/i << " "
        << ptrGlobal->perfG.tp/i << " " << ptrGlobal->perfG.tn/i << " " << ptrGlobal->perfG.fp/i << " " << ptrGlobal->perfG.fn/i << " "
        << ptrGlobal->perfB.tp/i << " " << ptrGlobal->perfB.tn/i << " " << ptrGlobal->perfB.fp/i << " " << ptrGlobal->perfB.fn/i << " "
        << ptrGlobal->metricR.sensitivity/i << " " << ptrGlobal->metricR.specificity/i << " " 
        << ptrGlobal->metricG.sensitivity/i << " " << ptrGlobal->metricG.specificity/i << " " 
        << ptrGlobal->metricB.sensitivity/i << " " << ptrGlobal->metricB.specificity/i ;
    return str.str();

}

string Performance::metricsStatisticsAsString() const
{
    stringstream str;
    str << stat.MeanR.sensitivity    << " " << stat.MeanR.specificity    << " "
        << stat.MedianR.sensitivity  << " " << stat.MedianR.specificity  << "   "
        << stat.MeanG.sensitivity    << " " << stat.MeanG.specificity    << " "
        << stat.MedianG.sensitivity  << " " << stat.MedianG.specificity  << "   "
        << stat.MeanB.sensitivity    << " " << stat.MeanB.specificity    << " "
        << stat.MedianB.sensitivity  << " " << stat.MedianB.specificity;
    return str.str();
}


// This calcualte indexes just for first channel
void Performance::evaluatePerformanceContingencyMatrix()
{
    this->evaluatePerformanceContingencyMatrixPerChannel(0);
}

void Performance::evaluatePerformanceContingencyMatrixPerChannel(int ch)
{
    if (measure == 0 || ch > MAX_NUMBER_CHANNELS) {
        sensitivity = 0;
        specificity = 0;
        precision   = 0;
        common_metrics = CommonMetrics(0,0,0);
    }
    else {
        sensitivity = measure[ch].tp/(measure[ch].tp + measure[ch].fn);
        specificity = measure[ch].fp/(measure[ch].tn + measure[ch].fp);
        precision   = measure[ch].tp/(measure[ch].tp + measure[ch].fp);
        common_metrics = CommonMetrics(sensitivity,specificity,precision);
    }

}

float Performance::getSensitivity(int ch)
{

    this->evaluatePerformanceContingencyMatrixPerChannel(ch);
    return sensitivity;
}

float Performance::getSpecificity(int ch)
{

    this->evaluatePerformanceContingencyMatrixPerChannel(ch);
    return specificity;
}
float Performance::getPrecision(int ch)
{

    this->evaluatePerformanceContingencyMatrixPerChannel(ch);
    return precision;
}



void Performance::pixelLevelCompare(const Mat& imref, const Mat& imcmp)
{
    //double duration;
    //duration = static_cast<double>(cv::getTickCount());

    if ( imref.dims > 2 || imcmp.dims > 2 ) {
        cerr << "Invalid dimensions :  " << imref.dims << " " << imcmp.dims << endl;
        return;
    }

    if ( imref.size() != imcmp.size() ) {
        cerr << "Invalid size :  " << imref.size() << " " << imcmp.size() << endl;
        return;
    }
    if ( imref.type() != imcmp.type() ) {
        cerr << "Invalid type :  " 
            << imref.type() << " " << imref.depth() << " " << imref.channels() <<  "  "
            << imcmp.type() << " " << imcmp.depth() << " " << imcmp.channels() <<  endl;
        return;
    }
    
    //Get TP and TN of reference and save them in refImageArray
    this->setPixelsReference(imref);
    
    unsigned int channels = imref.channels();
    nchannel = channels;
    int nl= imref.rows; // number of lines
    int nc= imref.cols;

    if ( imref.isContinuous() && imcmp.isContinuous() )
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }
    
    ContingencyMatrix *m_ptr = new ContingencyMatrix[channels];

    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        const uchar* imref_data= imref.ptr<uchar>(j);
        const uchar* imcmp_data= imcmp.ptr<uchar>(j);
        
        for (int i=0; i<nc; i++) {
            
            if (channels == 1) {
                //convert images to 0 or 1, binary form
                // 1: white 0: black
                uchar uref = imref_data[i]/150;
                uchar ucmp = imcmp_data[i]/150;
                
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
            else {
                for (int k=0; k<channels; k++) {
                    //convert images to 0 or 1, binary form
                    // 1: white
                    // 0: black
                    uchar uref = imref_data[i*channels+k]/150;
                    uchar ucmp = imcmp_data[i*channels+k]/150;
                    if (uref == ucmp) {
                        //silhouette, foreground pixel
                        if (uref)
                            (m_ptr+k)->tp++;
                        else
                            //background
                            (m_ptr+k)->tn++;
                    }
                    else {
                        if (uref)
                            (m_ptr+k)->fn++;
                        else
                            (m_ptr+k)->fp++;
                    }
                }
            }
            
        } // end of line
    }
    

    // Assign results to measure
    for (int i=0; i<channels; i++)
        measure[i] = m_ptr[i];

   
    //calculate sensitivity (TPR) and specificity (1-FPR)
    //puts result in common_metrics variable
    //just for first channel
    CommonMetrics _mR,_mG,_mB;
    this->evaluatePerformanceContingencyMatrix();
    _mR = common_metrics;

    GlobalMetrics *ptrGlobal = &global_metrics;
    ptrGlobal->perfR += m_ptr[0];
    ptrGlobal->metricR += common_metrics;
    ptrGlobal->count += 1;
    if (channels == 3) {
        ptrGlobal->perfG += m_ptr[1];
        ptrGlobal->perfB += m_ptr[2];
        this->evaluatePerformanceContingencyMatrixPerChannel(1);
        ptrGlobal->metricG += common_metrics;
        _mG = common_metrics;
        this->evaluatePerformanceContingencyMatrixPerChannel(2);
        ptrGlobal->metricB += common_metrics;
        _mB = common_metrics;
    }

    // Save each measure in a global vector
    vectorMetrics.push_back(
            GlobalMetrics(m_ptr[0],
                          m_ptr[1],
                          m_ptr[2],
                          _mR,
                          _mG,
                          _mB,
                          ptrGlobal->count));


    delete[] m_ptr;
    m_ptr=0;


    //duration = static_cast<double>(cv::getTickCount())-duration;
    //duration /= cv::getTickFrequency(); //the elapsed time in ms
}

// Counts up from reference frame number of TP and TN
void Performance::setPixelsReference(const Mat& image) 
{
    unsigned int channels = image.channels();

    ContingencyMatrix *r_ptr = new ContingencyMatrix[channels];

    int nl= image.rows; // number of lines
    int nc= image.cols;

    if (image.isContinuous())
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }

    //this loop is executed only once
    // in case of continuous images
    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        const uchar* data= image.ptr<uchar>(j);
        for (int i=0; i<nc; i++) {

            if (channels == 1) {
                if (data[i] > threshold)
                    r_ptr->tp++;
                else
                    r_ptr->tn++;
            }
            else {
                for (int k=0; k<channels; k++) {
                    if (data[i*channels + k] > threshold)
                        (r_ptr+k)->tp++;
                    else
                        (r_ptr+k)->tn++;
                }
            }

        } // end of line
    }

    for (int i=0; i<channels; i++)
        refImageArray[i] = r_ptr[i];
    delete[] r_ptr;
    r_ptr=0;
}

void Performance::meanOfMetrics() 
{
    if (global_metrics.count == 0)
        return;

    GlobalMetrics *g_p = &global_metrics;
    unsigned int cnt = g_p->count;

    stat.MeanR = CommonMetrics( g_p->metricR.sensitivity/cnt, g_p->metricR.specificity/cnt, 0 );
    stat.MeanG = CommonMetrics( g_p->metricG.sensitivity/cnt, g_p->metricG.specificity/cnt, 0 );
    stat.MeanB = CommonMetrics( g_p->metricB.sensitivity/cnt, g_p->metricB.specificity/cnt, 0 );

}

void Performance::medianOfMetrics() 
{
    vector<double> sen;
    vector<double> spe;
    
    vector<double> senG;
    vector<double> speG;
    vector<double> senB;
    vector<double> speB;

    unsigned int i = (unsigned int)vectorMetrics.size()/2; 
    bool even      = vectorMetrics.size()%2 ? false: true;

    for(vector<GlobalMetrics>::iterator it = vectorMetrics.begin(); 
                                        it != vectorMetrics.end(); ++it) 
    {
        sen.push_back(it->metricR.sensitivity);
        spe.push_back(it->metricR.specificity);

        if (nchannel > 1) {
            senG.push_back(it->metricG.sensitivity);
            speG.push_back(it->metricG.specificity);

            senB.push_back(it->metricB.sensitivity);
            speB.push_back(it->metricB.specificity);
        }
    }

    sort (sen.begin(), sen.end());
    sort (spe.begin(), spe.end());

    if (nchannel > 1) {
        sort (senG.begin(), senG.end());
        sort (speG.begin(), speG.end());
        sort (senB.begin(), senB.end());
        sort (speB.begin(), speB.end());
    }

    if (even) {
        stat.MedianR = CommonMetrics( (sen[i]+sen[i+1])/2,(spe[i]+spe[i+1])/2,0);
        if (nchannel > 1) {
            stat.MedianG = CommonMetrics( (senG[i]+senG[i+1])/2,(speG[i]+speG[i+1])/2,0);
            stat.MedianB = CommonMetrics( (senB[i]+senB[i+1])/2,(speB[i]+speB[i+1])/2,0);
        }
    } else {
        stat.MedianR = CommonMetrics( sen[i+1],spe[i+1],0);
        if (nchannel > 1) {
            stat.MedianG = CommonMetrics( senG[i+1],speG[i+1],0);
            stat.MedianB = CommonMetrics( senB[i+1],speB[i+1],0);
        }
    }

    /*
    if (even) { //even
        CommonMetrics( (sen[i]+sen[i+1])/2,(spe[i]+spe[i+1])/2,0);

        medianR[0] = (sen[i] + sen[i+1])/2;
        medianR[1] = (spe[i] + spe[i+1])/2;
    }
    else { //odd return number in the middle
        medianR[0] = sen[i+1];
        medianR[1] = spe[i+1];
    }

    cout << medianR[0] << " " << medianR[1] << endl;

    StatMetrics stat;
    */
}

void Performance::calculateFinalPerformanceOfMetrics()
{
    this->medianOfMetrics();
    this->meanOfMetrics();
}

}


