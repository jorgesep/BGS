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
    str << ref[0].tp << " " << ref[0].tn << " " << ref[1].tp << " " 
        << ref[1].tn << " " << ref[2].tp << " " << ref[2].tn << " "
        << measure[0].tp << " " << measure[0].tn << " " << measure[0].fp << " " << measure[0].fn << " "
        << measure[1].tp << " " << measure[1].tn << " " << measure[1].fp << " " << measure[1].fn << " "
        << measure[2].tp << " " << measure[2].tn << " " << measure[2].fp << " " << measure[2].fn << " ";
    //<< endl;
    return str.str();
}

string Performance::refToString() const
{
    stringstream str;
    str << ref[0].tp << " " << ref[0].tn << " " 
        << ref[1].tp << " " << ref[1].tn << " "
        << ref[2].tp << " " << ref[2].tn << " "
        << endl;
    return str.str();

}

/*    
float Performance::sensitivity()
{
    
    float Positive = measure + FalseNegative;
    if (Positive == 0) return 0;

    return (TruePositive/Positive);

}

float Performance::specificity()
{

    float Negative = FalsePositive + TrueNegative;
    if (Negative == 0) return 0;
    return (TrueNegative/Negative);
}

float Performance::precision()
{
    float Positive = TruePositive + FalsePositive;
    if (Positive == 0 ) return -1.0;
    return (TruePositive/Positive);
}
*/
    
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
    
    //Indexes *m_ptr = measure;
    Indexes *m_ptr = new Indexes[channels];

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
    
    delete[] m_ptr;
    m_ptr=0;
    
   
    //duration = static_cast<double>(cv::getTickCount())-duration;
    //duration /= cv::getTickFrequency(); //the elapsed time in ms
    
}

// Counts up from reference frame number of TP and TN
void Performance::setPixelsReference(const Mat& image) 
{
    unsigned int channels = image.channels();

    Indexes *r_ptr = new Indexes[channels];
    Indexes *refp = ref;

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
                    ref[0].tp++;
                else
                    ref[0].tn++;
            }
            else {
                for (int k=0; k<channels; k++) {
                    //cout << hex << data[i*channels+k] << endl;
                    if (data[i*channels + k] > threshold)
                        ref[k].tp++;
                    else
                        ref[k].tn++;
                }
            }

        } // end of line
    }

    delete[] r_ptr;
    r_ptr=0;
}


}


