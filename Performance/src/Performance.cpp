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
    //str << ref[0].tp << " " << ref[0].tn << " " << ref[1].tp << " " 
    //    << ref[1].tn << " " << ref[2].tp << " " << ref[2].tn << " "
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
    str << ref[0].tp << " " << ref[0].tn << " " 
        << ref[1].tp << " " << ref[1].tn << " "
        << ref[2].tp << " " << ref[2].tn;
    return str.str();

}

string Performance::summaryAsString() const
{
    stringstream str;

    cout << "AQUI" << endl;
    const GlobalMetrics *ptrGlobal = &global_metrics;
    unsigned int i = ptrGlobal->count; 
    str << i << " 0 0 0 0 0 0 " 
        << ptrGlobal->perfR.tp << " " << ptrGlobal->perfR.tn << " " << ptrGlobal->perfR.fp << " " << ptrGlobal->perfR.fn << " "
        << ptrGlobal->perfG.tp << " " << ptrGlobal->perfG.tn << " " << ptrGlobal->perfG.fp << " " << ptrGlobal->perfG.fn << " "
        << ptrGlobal->perfB.tp << " " << ptrGlobal->perfB.tn << " " << ptrGlobal->perfB.fp << " " << ptrGlobal->perfB.fn << " "
        << ptrGlobal->metricR.sensitivity << " " << ptrGlobal->metricR.specificity << " " 
        << ptrGlobal->metricG.sensitivity << " " << ptrGlobal->metricG.specificity << " " 
        << ptrGlobal->metricB.sensitivity << " " << ptrGlobal->metricB.specificity ;
    cout << "AQUI" << endl;
    return str.str();

}

string Performance::averageSummaryAsString() const
{
    stringstream str;

    cout << "ALLA" << endl;
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

    //if( src.dims <= 2 && dst.dims <= 2 && src.size() == dst.size() && src.type() == dst.type() ) {
    //if( imref.dims > 2 || imcmp.dims > 2 || imref.size() != imcmp.size() || imref.type() != imcmp.type() ) {

    //    cerr << "Invalid images, check dimension, sizes or types of both matrices." << endl;

    //    return;
    //}
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
             //<< imref.type() << " " << imref.depth() << " " << imref.channels << " "
             //<< imcmp.type() << " " << imcmp.depth() << " " << imcmp.channels << endl;
        return;
    }
    
    //Get TP and TN
    this->setPixelsReference(imref);
    
    unsigned int channels = imref.channels();
    int nl= imref.rows; // number of lines
    int nc= imref.cols;

    if ( imref.isContinuous() && imcmp.isContinuous() )
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }
    
    //ContingencyMatrix *m_ptr = measure;
    ContingencyMatrix *m_ptr = new ContingencyMatrix[channels];

    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        const uchar* imref_data= imref.ptr<uchar>(j);
        const uchar* imcmp_data= imcmp.ptr<uchar>(j);
        
        for (int i=0; i<nc; i++) {
            
            if (channels == 1) {
                //convert images to 0 or 1, binary form
                // 1: white
                // 0: black
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
    

    for (int i=0; i<channels; i++)
        measure[i] = m_ptr[i];

   
    //calculate sensitivity (TPR) and specificity (1-FPR)
    //just first channel
    this->evaluatePerformanceContingencyMatrix();

    GlobalMetrics *ptrGlobal = &global_metrics;
    ptrGlobal->perfR += m_ptr[0];
    ptrGlobal->metricR += common_metrics;
    ptrGlobal->count += 1;
    if (channels == 3) {
        ptrGlobal->perfG += m_ptr[1];
        ptrGlobal->perfB += m_ptr[2];
        this->evaluatePerformanceContingencyMatrixPerChannel(1);
        ptrGlobal->metricG += common_metrics;
        this->evaluatePerformanceContingencyMatrixPerChannel(2);
        ptrGlobal->metricB += common_metrics;
    }

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
            //int index = i * channels;

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
        ref[i] = r_ptr[i];
    delete[] r_ptr;
    r_ptr=0;
}


}


