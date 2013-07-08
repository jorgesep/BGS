/*//Implementation of the Gaussian mixture model background subtraction from:
//
//"Self-Adaptive Gaussian Mixture Model for urban traffic monitoring"
// Sezhi Shen
//Example usage with as cpp class
// BackgroundSubtractorMOG3 bg_model;
//For each new image the model is updates using:
// bg_model(img, fgmask);
//
//Example usage as part of the CvBGStatModel:
// CvBGStatModel* bg_model = cvCreateGaussianBGModel2( first_frame );
//
// //update for each frame
// cvUpdateBGStatModel( tmp_frame, bg_model );//segmentation result is in bg_model->foreground
//
// //release at the program termination
// cvReleaseBGStatModel( &bg_model );
//
//////////
//*/

#include <iomanip> 
#include <fstream>
//#include <sstream>
#include <algorithm>
#include "background_subtraction.h"

#include <opencv2/opencv.hpp>


#include "precomp.h"
#include "icdm_model.h"

using namespace std;
using namespace cv;

// default parameters of gaussian background detection algorithm
static const int   defaultHistory2         = 500; // Learning rate; alpha = 1/defaultHistory2
static const float defaultVarThreshold2    = 4.0f*4.0f;
static const int   defaultNMixtures2       = 5; // maximal number of Gaussians in mixture
static const float defaultBackgroundRatio2 = 0.9f; // threshold sum of weights for background test
static const float defaultVarThresholdGen2 = 3.0f*3.0f;
static const float defaultVarInit2         = 15.0f; // initial variance for new components
static const float defaultVarMax2          = 5*defaultVarInit2;
static const float defaultVarMin2          = 4.0f;

// additional parameters
static const float defaultfCT2             = 0.05f; // complexity reduction prior constant 0 - no reduction of number of components
static const float defaultfTau             = 0.5f; // Tau - shadow threshold, see the paper for explanation
static const unsigned char defaultnShadowDetection2 = (unsigned char)127; // value to use in the segmentation mask for shadows, set 0 not to do shadow detection


const float BackgroundSubtractorMOG3::Alpha       = 0.0001f; //speed of update, the time interval =1/Alfa.
const float BackgroundSubtractorMOG3::Cf          = 0.05f;
const int   BackgroundSubtractorMOG3::GaussiansNo = 4;       // Max. number of Gaussian per pixel
const float BackgroundSubtractorMOG3::Sigma       = 11.0f;    // Initial sigma.
const float BackgroundSubtractorMOG3::SigmaMax    = 5.0f*BackgroundSubtractorMOG3::Sigma;    // Initial sigma.
const float BackgroundSubtractorMOG3::SigmaMin    = 4.0f;    // Initial sigma.
const float BackgroundSubtractorMOG3::T           = 1.0f - BackgroundSubtractorMOG3::Cf;
const float BackgroundSubtractorMOG3::PixelRange  = 4.0f*4.0f; // A threshold to decide if a pixel value match one of a existent Gaussian
const float BackgroundSubtractorMOG3::PixelGen    = 3.0f*3.0f; // Normally, Gen < Range
const float BackgroundSubtractorMOG3::CT          = 0.05f;
const float BackgroundSubtractorMOG3::Tau         = 0.5f ;




struct GaussBGStatModel2Params
{
    //image info
    int nWidth;
    int nHeight;
    int nND;//number of data dimensions (image channels)

    bool bPostFiltering;//defult 1 - do postfiltering - will make shadow detection results also give value 255
    double  minArea; // for postfiltering

    bool bInit;//default 1, faster updates at start

    /////////////////////////
    //very important parameters - things you will change
    ////////////////////////
    float fAlphaT;
    //alpha - speed of update - if the time interval you want to average over is T
    //set alpha=1/T. It is also usefull at start to make T slowly increase
    //from 1 until the desired T
    float fTb;
    //Tb - threshold on the squared Mahalan. dist. to decide if it is well described
    //by the background model or not. Related to Cthr from the paper.
    //This does not influence the update of the background. A typical value could be 4 sigma
    //and that is Tb=4*4=16;

    /////////////////////////
    //less important parameters - things you might change but be carefull
    ////////////////////////
    float fTg;
    //Tg - threshold on the squared Mahalan. dist. to decide
    //when a sample is close to the existing components. If it is not close
    //to any a new component will be generated. I use 3 sigma => Tg=3*3=9.
    //Smaller Tg leads to more generated components and higher Tg might make
    //lead to small number of components but they can grow too large
    float fTB;//1-cf from the paper
    //TB - threshold when the component becomes significant enough to be included into
    //the background model. It is the TB=1-cf from the paper. So I use cf=0.1 => TB=0.
    //For alpha=0.001 it means that the mode should exist for approximately 105 frames before
    //it is considered foreground
    float fVarInit;
    float fVarMax;
    float fVarMin;
    //initial standard deviation  for the newly generated components.
    //It will will influence the speed of adaptation. A good guess should be made.
    //A simple way is to estimate the typical standard deviation from the images.
    //I used here 10 as a reasonable value
    float fCT;//CT - complexity reduction prior
    //this is related to the number of samples needed to accept that a component
    //actually exists. We use CT=0.05 of all the samples. By setting CT=0 you get
    //the standard Stauffer&Grimson algorithm (maybe not exact but very similar)

    //even less important parameters
    int nM;//max number of modes - const - 4 is usually enough

    //shadow detection parameters
    bool bShadowDetection;//default 1 - do shadow detection
    unsigned char nShadowDetection;//do shadow detection - insert this value as the detection result
    float fTau;
    // Tau - shadow threshold. The shadow is detected if the pixel is darker
    //version of the background. Tau is a threshold on how much darker the shadow can be.
    //Tau= 0.5 means that if pixel is more than 2 times darker then it is not shadow
    //See: Prati,Mikic,Trivedi,Cucchiarra,"Detecting Moving Shadows...",IEEE PAMI,2003.
};

struct GMM
{
    float weight;
    float variance;
    
    GMM()
    :weight(1.0f),variance(11.0f) {}
    
    GMM(float _w, float _v)
    :weight(_w),variance(_v) {}
    
    //copy constructor
    GMM (const GMM &_m) { *this = _m; }
    
    GMM & operator = (const GMM & rhs) {
        if ( *this != rhs ) {
            weight   = rhs.weight;
            variance = rhs.variance;
        }
        return *this;
    }
    
    bool operator != (const GMM & rhs) const {
        return (weight != rhs.weight) || (variance != rhs.variance);
    }
    
    bool operator == (const GMM & rhs) const {
        return (weight == rhs.weight) && (variance == rhs.variance);
    }   

};

struct MEAN 
{
    float meanR;
    float meanG;
    float meanB;
    
    MEAN (float _r, float _g, float _b) 
    : meanR(_r), meanG(_g), meanB(_b) { } 
    
    //copy constructor
    MEAN (const MEAN &_m) { *this = _m; }
    
    MEAN & operator = (const MEAN & rhs) {
        if ( *this != rhs ) {
            meanR = rhs.meanR;
            meanG = rhs.meanG;
            meanB = rhs.meanB;
        }
        return *this;
    }
    
    bool operator != (const MEAN & rhs) const {
        return 
        (meanR != rhs.meanR) ||
        (meanG != rhs.meanG) ||
        (meanB != rhs.meanB); 
    }
    
    bool operator == (const MEAN & rhs) const {
        return 
        (meanR == rhs.meanR) &&
        (meanG == rhs.meanG) && 
        (meanB == rhs.meanB); 
    }   
};

// shadow detection performed per pixel
// should work for rgb data, could be usefull for gray scale and depth data as well
// See: Prati,Mikic,Trivedi,Cucchiarra,"Detecting Moving Shadows...",IEEE PAMI,2003.
static CV_INLINE bool
detectShadowGMM(const float* data, int nchannels, int nmodes,
                const GMM* gmm, const float* mean,
                float Tb, float TB, float tau, float _globalChange)
{
    float tWeight = 0;
    float globalChange = _globalChange;
    //globalChange = 1.0;

    // check all the components  marked as background:
    for( int mode = 0; mode < nmodes; mode++, mean += nchannels )
    {
        GMM g = gmm[mode];

        float numerator = 0.0f;
        float denominator = 0.0f;
        for( int c = 0; c < nchannels; c++ )
        {
            //numerator   += data[c] * mean[c];
            numerator   += data[c] * mean[c] * globalChange;
            denominator += mean[c] * mean[c];
        }

        // no division by zero allowed
        if( denominator == 0 )
            return false;

        // if tau < a < 1 then also check the color distortion
        if( numerator <= denominator && numerator >= tau*denominator )
        {
            //float a = numerator / denominator;
            float shadowAlfa = numerator / denominator;
            float dist2a = 0.0f;

            for( int c = 0; c < nchannels; c++ )
            {
                //float dD= a*mean[c] - data[c];
                float dD= shadowAlfa * mean[c] - data[c] * globalChange;
                dist2a += dD*dD;
            }

            if (dist2a < Tb*g.variance * shadowAlfa * shadowAlfa)
                return true;
        };

        tWeight += g.weight;
        if( tWeight > TB )
            return false;
    };
    return false;
}

class BackgroundSubtractionInvoker : public ParallelLoopBody
{
public:    
    BackgroundSubtractionInvoker(
                                const Mat& _src, 
                                Mat& _dst,
                                GMM* _gmm, 
                                float* _mean,
                                uchar* _modesUsed,
                                int _nmixtures, 
                                float _alphaT,
                                float _Tb, 
                                float _TB, float _Tg,
                                float _varInit,
                                float _varMin,
                                float _varMax,
                                float _prune,
                                float _tau,
                                bool _detectShadows,
                                uchar _shadowVal,
                                float _globalChange,
                                float* _Cm,
                                float* _Bg,float* _Fg, Point _debugPt) 
{
    src = &_src;
    dst = &_dst;
    gmm0 = _gmm;
    mean0 = _mean;
    modesUsed0 = _modesUsed;
    nmixtures = _nmixtures;
    alphaT = _alphaT;
    Tb = _Tb;
    TB = _TB;
    Tg = _Tg;
    varInit = _varInit;
    varMin = MIN(_varMin, _varMax);
    varMax = MAX(_varMin, _varMax);
    prune = _prune;
    tau = _tau;
    detectShadows = _detectShadows;
    shadowVal = _shadowVal;
    
    globalChange = _globalChange;
    Cm0 = _Cm;
    Bg0 = _Bg;
    //Bg0 = _Fg;
    Fg0 = _Fg;

    debugPt = _debugPt;
    
    cvtfunc = src->depth() != CV_32F ? getConvertFunc(src->depth(), CV_32F) : 0;
}

    
void operator()(const Range& range) const
{
    int y0 = range.start;
    int y1 = range.end;
        
    int ncols     = src->cols;
    int nchannels = src->channels();
    
    AutoBuffer<float> buf(src->cols*nchannels);
    
    float alpha1 = 1.f - alphaT;
    float dData[CV_CN_MAX];

    //run for each row
    for( int y = y0; y < y1; y++ )
    {
        const float* data = buf;//data is pointer, which points to const float
        if( cvtfunc )
            cvtfunc( src->ptr(y), src->step, 0, 0, (uchar*)data, 0, Size(ncols*nchannels, 1), 0);
        else
            data = src->ptr<float>(y);

        //'y' select number of row
        float* mean      = mean0 + ncols*nmixtures*nchannels*y;
        GMM*   gmm       = gmm0  + ncols*nmixtures*y;
        uchar* modesUsed = modesUsed0 + ncols*y;
        uchar* mask      = dst->ptr(y);
        float* cm        = Cm0 + ncols*nmixtures*y;
        float* bg        = Bg0 + ncols*nchannels*y;
        float* fg        = Fg0 + ncols*y;

        //After each iteration per mixture:
        // increment x (note: x is a column)
        // data (buffer) incremented by number of channels.
        // ggm incremented number of Gaussians
        // data:
        //
        // |--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
        // |B |G |R |  |  |  |  |  |  |  |  |  |  |  |  |  |
        // |--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
        //
        for( int x = 0; x < ncols; x++, data += nchannels, gmm += nmixtures, mean += nmixtures*nchannels, cm+=nmixtures, bg+=nchannels )
        {
            //calculate distances to the modes (+ sort)
            //here we need to go in descending order!!!
            bool background   = false;//return value -> true - the pixel classified as background

            //internal:
            bool fitsPDF      = false;//if it remains zero a new GMM mode will be added
            int nmodes        = modesUsed[x];
            int nNewModes     = nmodes;//current number of modes in GMM
            float totalWeight = 0.f;

            float* mean_m     = mean;
            float* bg_m       = bg;
            float* fg_m       = fg;
            float* bg_cnt     = cm;

            //just for debugging
            if (y0==228 && x==670)
            //if (y0==200 && x==680)
            //if (y0==160 && x==400)    
                int temporary = y0;
            

            //////
            //go through all modes
            //for( int mode = 0; mode < nmodes; mode++, mean_m += nchannels, bg_m +=nchannels )
            for( int mode = 0; mode < nmodes; mode++, mean_m += nchannels)
            {
                // prune = -learningRate*fCT = 1./500*0.05 = -0.0001
                // Ownership Om set zero to obtain weight if fit is not found.
                // Eq (14) ownership in zero
                float weight = alpha1*gmm[mode].weight + prune;//need only weight if fit is found
                
                //// 
                //fit not found yet, at init fitsPDF <-- false
                if( !fitsPDF )
                {
                    //check if it belongs to some of the remaining modes
                    float var = gmm[mode].variance;

                    //calculate difference and distance
                    float dist2;

                    // dData[CV_CN_MAX] == dData[512]
                    // d_dirac_m = x[t] - mu_m
                    if( nchannels == 3 )
                    {
                        //just for debugging
                        /*
                        float mean0 = mean_m[0];
                        float mean1 = mean_m[1];
                        float mean2 = mean_m[2];
                        float data0 = data[0];
                        float data1 = data[1];
                        float data2 = data[2];
                        data0 *=globalChange;
                        data1 *=globalChange;
                        data2 *=globalChange;
                        dData[0] = mean0 - data0;
                        dData[1] = mean1 - data1;
                        dData[2] = mean2 - data2;
                        */
                        dData[0] = mean_m[0] - data[0]*globalChange;
                        dData[1] = mean_m[1] - data[1]*globalChange;
                        dData[2] = mean_m[2] - data[2]*globalChange;
                        
                        dist2 = dData[0]*dData[0] + dData[1]*dData[1] + dData[2]*dData[2];
                    }
                    else
                    {
                        dist2 = 0.f;
                        for( int c = 0; c < nchannels; c++ )
                        {
                            dData[c] = mean_m[c] - data[c]*globalChange;
                            dist2 += dData[c]*dData[c];
                        }
                    }

                    // Eq (8)
                    // SUM(Weight) > (1-Cf); TB=(1-Cf)
                    //background? - Tb - usually larger than Tg
                    if( totalWeight < TB && dist2 < Tb*var ) {
                        background = true;
                        
                        for( int c = 0; c < nchannels; c++ )
                            bg_m[c] = mean_m[c];
                            //bg_m[c] = dData[c];
                        
                    }

                    //check fit
                    if( dist2 < Tg*var )
                    {
                        /////
                        //belongs to the mode
                        fitsPDF = true;

                        //update distribution
                        //modesUsed[mode] = mode + 1;
                        
                        //Increment background counter number
                        bg_cnt[mode] += 1;
                                               
                        //New Beta dynamic learning rate, se eq. 4.7
                        //Beta=alfa(h+Cm)/Cm
                        //If the background changes quickly, Cm will become smaller, new beta learning rate will increase
                        //
                        //just for debugging
                        //double aT    = (double)alphaT;
                        //double bgcnT = (double)bg_cnt[mode];
                        //double Beta  = aT/bgcnT;
                        //Beta += aT;
                        
                        float Beta = alphaT/bg_cnt[mode]+alphaT;
                        //see eq. (4.8) (4.9)
                        float k = Beta/gmm[mode].weight;
                        
                        // Update Weight
                        // Eq (14) of Zivkovic paper
                        // prune = -learningRate*fCT
                        //weight = alpha1*gmm[mode].weight+alphaT + prune;
                        weight += alphaT;
                        
                        // Update mean
                        // Eq (5) 
                        // TODO: Check for more than three channels
                        for( int c = 0; c < nchannels; c++ )
                            mean_m[c] -= k*dData[c];
                        
                        // Eq(6)
                        // update variance
                        float varnew = var + k*(dist2-var);
                        //limit the variance
                        varnew = MAX(varnew, varMin);
                        varnew = MIN(varnew, varMax);
                        gmm[mode].variance = varnew;

                         
                        //sort
                        //all other weights are at the same place and
                        //only the matched (iModes) is higher -> just find the new place for it
                        for( int i = mode; i > 0; i-- )
                        {
                            //check one up
                            if( weight < gmm[i-1].weight )
                                break;

                            //swap one up
                            std::swap(gmm[i], gmm[i-1]);
                            std::swap(cm[i], cm[i-1]);
                            for( int c = 0; c < nchannels; c++ )
                                std::swap(mean[i*nchannels + c], mean[(i-1)*nchannels + c]);
                        }
                        //belongs to the mode - bFitsPDF becomes 1
                        /////
                    }
                }//!bFitsPDF)

                //check prune
                if( weight < -prune )
                {
                    weight = 1.0E-6;
                    nmodes--;
                }

                gmm[mode].weight = weight;//update weight by the calculated value
                totalWeight += weight;
            }
            //go through all modes
            //////

            //renormalize weights
            totalWeight = 1.f/totalWeight;
            for( int mode = 0; mode < nmodes; mode++ )
                gmm[mode].weight *= totalWeight;

            nmodes = nNewModes;

            //make new mode if needed and exit
            if( !fitsPDF )
            {

                // replace the weakest or add a new one
                int mode = nmodes == nmixtures ? nmixtures-1 : nmodes++;

                if (nmodes==1)
                    gmm[mode].weight = 1.f;
                else
                {
                    gmm[mode].weight = alphaT;

                    // renormalize all other weights
                    for( int i = 0; i < nmodes-1; i++ )
                        gmm[i].weight *= alpha1;
                }

                // init
                for( int c = 0; c < nchannels; c++ )
                    mean[mode*nchannels + c] = data[c];

                gmm[mode].variance = varInit;
                bg_cnt[mode] = 1;

                //sort
                //find the new place for it
                for( int i = nmodes - 1; i > 0; i-- )
                {
                    // check one up
                    if( alphaT < gmm[i-1].weight )
                        break;

                    // swap one up
                    std::swap(gmm[i], gmm[i-1]);
                    std::swap(cm[i], cm[i-1]);
                    for( int c = 0; c < nchannels; c++ )
                        std::swap(mean[i*nchannels + c], mean[(i-1)*nchannels + c]);
                }
            }//make new mode if needed and exit

            //just for debugging save parameters to output file
            if ( debugPt != Point(0,0) ) {
                
                if (debugPt.x == x && debugPt.y == y0){
                    //if (y0==240 && x==670) {
                    //if (y0==228 && x==670) {
                    //if (y0==160 && x==400) {
                    //std::ofstream outfile;
                    //outfile.open("bg_params_670_240.txt",ios::out | ios::app);
                    std::cout.precision(5);
                    cout 
                    << debugPt.x << " " << debugPt.y << " "
                    << "mode: " << nmodes 
                    << " data: "   << (int)data[0]           << " " << (int)data[1]              << " " << (int)data[2]
                    << " bg_cnt: " << (int)bg_cnt[0]         << " " << (int)bg_cnt[1]            << " " << (int)bg_cnt[2]       << " " << (int)bg_cnt[3]
                    << " mean: "   << (int)mean[0]           << " " << (int)mean[1]              << " " << (int)mean[2] 
                    << " ; "       << (int)mean[1*nchannels] << " " << (int)mean[1*nchannels +1] << " " << (int)mean[1*nchannels +2] 
                    << " ; "       << (int)mean[2*nchannels] << " " << (int)mean[2*nchannels +1] << " " << (int)mean[2*nchannels +2] 
                    << " ; "       << (int)mean[3*nchannels] << " " << (int)mean[3*nchannels +1] << " " << (int)mean[3*nchannels +2] 
                    << " var: "    << gmm[0].variance        << " " << gmm[1].variance           << " " << gmm[2].variance       << " " << gmm[3].variance
                    << " weight: " << gmm[0].weight          << " " << gmm[1].weight              << " " << gmm[2].weight        << " " << gmm[3].weight
                    << endl;
                    //outfile.close();
                }
            }
            
            
            //set the number of modes
            //modesUsed[x] = uchar(nmodes);
            modesUsed[x] = (uchar)nmodes;
            mask[x] = background ? 0 :
                detectShadows && detectShadowGMM(data, nchannels, nmodes, gmm, mean, Tb, TB, tau, globalChange) ?
                shadowVal : 255;
        }//end columns for
    }
}

    const Mat* src;
    Mat* dst;
    GMM* gmm0;
    float* mean0;
    uchar* modesUsed0;

    int nmixtures;
    float alphaT, Tb, TB, Tg;
    float varInit, varMin, varMax, prune, tau;

    bool detectShadows;
    uchar shadowVal;

    float globalChange;
    float* Cm0;
    float* Bg0;
    float* Fg0;
    
    Point debugPt;
    
    BinaryFunc cvtfunc;
};


BackgroundSubtractorMOG3::BackgroundSubtractorMOG3()
{
    frameSize        = Size(0,0);
    frameType        = 0;
    
    nframes          = 0;
    history          = defaultHistory2; //500 --> 1/500 = 0.002
    varThreshold     = PixelRange;
    bShadowDetection = 1;
    
    nmixtures        = GaussiansNo;
    fAlpha           = Alpha;
    backgroundRatio  = T;
    fVarInit         = Sigma;
    fVarMax          = SigmaMax;
    fVarMin          = SigmaMin;
    
    varThresholdGen  = PixelGen;
    fCT              = CT;
    nShadowDetection =  defaultnShadowDetection2;
    fTau             = Tau;
    
    alreadyInitialized = false;
    pointToDebug = Point(0,0);

}


BackgroundSubtractorMOG3::BackgroundSubtractorMOG3(int _history,  float _varThreshold, bool _bShadowDetection)
{
    frameSize        = Size(0,0);
    frameType        = 0;

    nframes          = 0;
    history          = _history > 0 ? _history : defaultHistory2;
    varThreshold     = (_varThreshold>0)? _varThreshold : PixelRange;
    bShadowDetection = _bShadowDetection;

    nmixtures        = GaussiansNo;
    fAlpha           = Alpha;
    backgroundRatio  = T;
    fVarInit         = Sigma;
    fVarMax          = SigmaMax;
    fVarMin          = SigmaMin;

    varThresholdGen  = PixelGen;
    fCT              = CT;
    nShadowDetection =  defaultnShadowDetection2;
    fTau             = Tau;

    alreadyInitialized = false;
    pointToDebug = Point(0,0);

}

BackgroundSubtractorMOG3::BackgroundSubtractorMOG3(string init)
{
    frameSize        = Size(0,0);
    frameType        = 0;
    
    nframes          = 0;
    history          = defaultHistory2; //500 --> 1/500 = 0.002
    varThreshold     = PixelRange;
    bShadowDetection = 1;
    
    nmixtures        = GaussiansNo;
    fAlpha           = Alpha;
    backgroundRatio  = T;
    fVarInit         = Sigma;
    fVarMax          = SigmaMax;
    fVarMin          = SigmaMin;
    
    varThresholdGen  = PixelGen;
    fCT              = CT;
    nShadowDetection =  defaultnShadowDetection2;
    fTau             = Tau;

    initParametersName = init;
    loadInitParametersFromFile(init);

    alreadyInitialized = false;
    pointToDebug = Point(0,0);

}


BackgroundSubtractorMOG3::~BackgroundSubtractorMOG3()
{
}


void BackgroundSubtractorMOG3::initializeModel(InputArray _image)
{
    Mat image = _image.getMat();
    frameSize = image.size();
    frameType = image.type();
    frameDepth= image.depth();
    
    nframes = 0;
    
    int nchannels = CV_MAT_CN(frameType);
    CV_Assert( nchannels <= CV_CN_MAX );
        
    // number of lines
    int nl= frameSize.height; 
    int nc= frameSize.width;
    
    int matSize   = frameSize.height*frameSize.width;
    int columnsNo = matSize*nmixtures*(2 + nchannels);
    
    //Keep a result of background and foreground every call processing
    //Background.create(1, matSize*nchannels, frameDepth);
    Background.create(1, matSize*nchannels, CV_32F);
    //Background.create(1, matSize, CV_32FC(nchannels));
    Foreground.create(1, matSize, CV_32F);
    Background = Scalar::all(0);
    Foreground = Scalar::all(0);
    float* ptrBG  = (float *)Background.data;
    float* Bg     = ptrBG;

    
    // for each gaussian mixture of each pixel bg model we store ...
    // the mixture weight (w),
    // the mean (nchannels values) and
    // the covariance
    GaussianModel.create(1, columnsNo, CV_32F );
    GaussianModel = Scalar::all(0);
    
    GMM* ptrGMM = (GMM*)GaussianModel.data;
    GMM*   data  = ptrGMM;
    
    MEAN* ptrMean = (MEAN*)(GaussianModel.data + sizeof(GMM)*nmixtures*matSize);
    MEAN*  ptrm  = ptrMean;
    
    if ( image.isContinuous() )
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }
    
    
    for (int j=0; j<nl; j++) {
        for (int i=0; i<nc; i++) {
            
            data = ptrGMM + i*nmixtures + j*nmixtures*nc;
            data->weight     = 1.0f;
            data->variance = fVarInit;
            
            //increment pointer in a 'j' row and 'i' column
            ptrm = ptrMean + i*nmixtures + j*nmixtures*nc;
            ptrm->meanB = static_cast<float>(image.at<Vec3b>(j,i)[0]);
            ptrm->meanG = static_cast<float>(image.at<Vec3b>(j,i)[1]);
            ptrm->meanR = static_cast<float>(image.at<Vec3b>(j,i)[2]);
            
            //initialize background with current input image.
            Bg    = ptrBG + i*nchannels + j*nchannels*nc;
            Bg[0] = image.at<Vec3f>(j,i)[0];
            Bg[1] = image.at<Vec3f>(j,i)[1];
            Bg[2] = image.at<Vec3f>(j,i)[2];
            
            
        }
    }    
    /*
    cout
    << "(0,0): [" << *(ptrBG + 0) << "," << *(ptrBG + 1) << "," << *(ptrBG + 2) << "] "
    << "(0,1): [" << *(ptrBG + 3) << "," << *(ptrBG + 4) << "," << *(ptrBG + 5) << "] "
    << "(1,0): [" << *(ptrBG + 6) << "," << *(ptrBG + 7) << "," << *(ptrBG + 8) << "] "
    << "(1,1): [" << *(ptrBG + 9) << "," << *(ptrBG + 10) << "," << *(ptrBG + 11) << "] "
    << endl;
    */
    
    CurrentGaussianModel.create(frameSize, CV_8U);
    CurrentGaussianModel = Scalar::all(0);
    
    
    BackgroundNumberCounter.create(1, frameSize.height*frameSize.width*nmixtures, CV_32F);
    BackgroundNumberCounter = Scalar::all(0);
    float* ptrCnt = (float*)BackgroundNumberCounter.data;
    int ncols= BackgroundNumberCounter.cols * BackgroundNumberCounter.channels();
    for (int i=0; i<ncols; i+=nmixtures)
        ptrCnt[i]= 1.0f;
    
    
    
    alreadyInitialized = true;
    //outfile.open("internal_values.txt");
}



void BackgroundSubtractorMOG3::initialize(Size _frameSize, int _frameType)
{
    frameSize = _frameSize;
    frameType = _frameType;
    nframes = 0;

    int nchannels = CV_MAT_CN(frameType);
    CV_Assert( nchannels <= CV_CN_MAX );

    // for each gaussian mixture of each pixel bg model we store ...
    // the mixture weight (w),
    // the mean (nchannels values) and
    // the covariance
    //bgmodel.create( 1, frameSize.height*frameSize.width*nmixtures*(2 + nchannels), CV_32F );
    
    //make the array for keeping track of the used modes per pixel - all zeros at start
    //bgmodelUsedModes.create(frameSize,CV_8U);
    //bgmodelUsedModes = Scalar::all(0);
    
    
    int matSize   = frameSize.height*frameSize.width;
    int columnsNo = matSize*nmixtures*(2 + nchannels);
    
    GaussianModel.create(1, columnsNo, CV_32F );
    GMM* ptrGMM = (GMM*)GaussianModel.data;
    GMM*   data  = ptrGMM;
    
    //MEAN* ptrMean = (MEAN*)(GaussianModel.data + 2*nmixtures*matSize);
    MEAN* ptrMean = (MEAN*)(GaussianModel.data + sizeof(GMM)*nmixtures*matSize);
    MEAN*  ptrm  = ptrMean;
    
    //int size1= sizeof(GMM);
    //int size2 = sizeof(MEAN);
    
    for (int i=0; i<matSize; i++) {
        data = ptrGMM + i*nmixtures;
        data->weight     = 1.0f;
        data->variance = 11.0f;
        
        //initialize first gaussian mean (RGB)
        ptrm = ptrMean + i*nmixtures;
        ptrm->meanR = 1.0f;
        ptrm->meanG = 1.0f;
        ptrm->meanB = 1.0f;
    }
    //GaussianModel = Scalar::all(1);
    
    CurrentGaussianModel.create(frameSize, CV_8U);
    //CurrentGaussianModel = Scalar(1,0,0,0);
    CurrentGaussianModel = Scalar::all(0);
    
    
    BackgroundNumberCounter.create(1, frameSize.height*frameSize.width*nmixtures, CV_32F);
    BackgroundNumberCounter = Scalar::all(0);

    float* ptrCnt = (float*)BackgroundNumberCounter.data;
    int nc= BackgroundNumberCounter.cols * BackgroundNumberCounter.channels();
    for (int i=0; i<nc; i+=nmixtures)
        ptrCnt[i]= 1.0f;
    
    //Keep a result of background and foreground every call processing
    Background.create(1, matSize*nchannels, CV_32F);
    Foreground.create(1, matSize,           CV_32F);
    

    

}

void BackgroundSubtractorMOG3::operator()(InputArray _image, OutputArray _fgmask, double learningRate)
{
    Mat image = _image.getMat();
    bool needToInitialize = (nframes == 0 || 
                            learningRate >= 1 || 
                            image.size() != frameSize || 
                            image.type() != frameType) && !alreadyInitialized;

    if( needToInitialize )
        initialize(image.size(), image.type());

    _fgmask.create( image.size(), CV_8U );
    Mat fgmask = _fgmask.getMat();

    ++nframes;
    
    //Mat bg;
    //getBackgroundImage(bg);
    
    //learningRate = learningRate >= 0 && nframes > 1 ? learningRate : 1./min( 2*nframes, history );
    //learningRate = Alpha;
    learningRate = fAlpha;
    CV_Assert(learningRate >= 0);

    //Global illumination changing factor 'g' between reference image ir and current image ic.
    float globalIlluminationFactor = 1.0;
    
    Mat bg;
    getBackgroundImage(bg);
    //getBackground(bg);
    globalIlluminationFactor = icdm::Instance()->getIlluminationFactor(bg,image);
        
    if (globalIlluminationFactor > 2)
        globalIlluminationFactor = 2;
    //cout << "GLOBAL ILLUMINATION FACTOR: " << globalIlluminationFactor << endl;
    
    //globalIlluminationFactor = 1;
    
    BackgroundSubtractionInvoker invoker(
            image, 
            fgmask, 
            (GMM*)GaussianModel.data, 
            (float*)(GaussianModel.data + sizeof(GMM)*nmixtures*image.rows*image.cols),
            CurrentGaussianModel.data, 
            nmixtures, 
            (float)learningRate,
            (float)varThreshold,
            backgroundRatio, 
            varThresholdGen,
            fVarInit, 
            fVarMin, 
            fVarMax, 
            float(-learningRate*fCT), 
            fTau,
            bShadowDetection, 
            nShadowDetection,
            globalIlluminationFactor,
            (float *)BackgroundNumberCounter.data,
            (float *)Background.data, (float *)Foreground.data, pointToDebug);
    
    parallel_for_(Range(0, image.rows), invoker);

}

void BackgroundSubtractorMOG3::getBackgroundImage(OutputArray backgroundImage) const
{
    int nchannels = CV_MAT_CN(frameType);
    CV_Assert( nchannels == 3 );
    Mat meanBackground(frameSize, CV_8UC3, Scalar::all(0));

    int firstGaussianIdx = 0;
    const GMM* gmm = (GMM*)GaussianModel.data;
    const Vec3f* mean = reinterpret_cast<const Vec3f*>(gmm + frameSize.width*frameSize.height*nmixtures);

    for(int row=0; row<meanBackground.rows; row++)
    {
        for(int col=0; col<meanBackground.cols; col++)
        {
            int nmodes = CurrentGaussianModel.at<uchar>(row, col);
            Vec3f meanVal;
            float totalWeight = 0.f;
            for(int gaussianIdx = firstGaussianIdx; gaussianIdx < firstGaussianIdx + nmodes; gaussianIdx++)
            {
                GMM gaussian = gmm[gaussianIdx];
                meanVal += gaussian.weight * mean[gaussianIdx];
                totalWeight += gaussian.weight;

                if(totalWeight > backgroundRatio)
                    break;
            }

            meanVal *= (1.f / totalWeight);
            meanBackground.at<Vec3b>(row, col) = Vec3b(meanVal);
            firstGaussianIdx += nmixtures;
        }
    }

    switch(CV_MAT_CN(frameType))
    {
    case 1:
    {
        vector<Mat> channels;
        split(meanBackground, channels);
        channels[0].copyTo(backgroundImage);
        break;
    }

    case 3:
    {
        meanBackground.copyTo(backgroundImage);
        break;
    }

    default:
        CV_Error(CV_StsUnsupportedFormat, "");
    }
}

void BackgroundSubtractorMOG3::getBackground(OutputArray _bgImage)
{
    int nchannels = CV_MAT_CN(frameType);
    //CV_MAT_TYPE(<#flags#>)
    CV_Assert( nchannels == 3 );
    _bgImage.create(frameSize, CV_8UC3);

    // get Mat header.
    Mat bg0 = _bgImage.getMat();
    bg0     = Scalar::all(0);
    
    // number of lines
    int nl= bg0.rows; 
    int nc= bg0.cols;
    
    if ( Background.isContinuous() && bg0.isContinuous() )
    {
        //then no padded pixels
        nc= nc*nl;
        nl= 1; // it is now a 1D array
    }

    const float* bg1_data;
    uchar* bg0_data;
    
    for (int j=0; j<nl; j++) {
        //get a pointer of each row
        bg1_data = Background.ptr<float>(j);
        bg0_data = bg0.ptr<uchar>(j);
        
        for (int i=0; i<nc*nchannels; i++) {
            uchar tmp = static_cast<uchar>(bg1_data[i]);
            //bg0_data[i] = (uchar)bg1_data[i];
            bg0_data[i] = tmp;
        }
    }

    
}

void BackgroundSubtractorMOG3::loadBackgroundModelFromFile(const string bgModelInputName)
{

}

void BackgroundSubtractorMOG3::saveBackgroundModelToFile(const string bgModelOutputName)
{

}

void BackgroundSubtractorMOG3::loadInitParametersFromFile(const string initInputParameters = "init.txt")
{


    ifstream file(initInputParameters.c_str());
    if (!file.good()) 
        return ;

    //Logging initialization
    //I'd like to use a logging tool, like boost-log to send all debug messages.
    //But just for now 'cout' seems to be good enough.
    //cout << "loadInitParametersFromFile : " << initInputParameters << endl;
    string line;
    while(getline(file, line)) {
        if(!line.length() || line[0] == '#') continue;
        else {
            //Removing spaces
            string::iterator end_pos = std::remove(line.begin(), line.end(), ' ');
            line.erase(end_pos, line.end());
            size_t pos = line.find(":");
            size_t end = line.size() - pos;

            if (pos != string::npos) {
                stringstream strval( line.substr( pos+1,end ) );

                if      (line.substr(0,pos) == "GaussiansNo" )
                    strval >> nmixtures;
                else if (line.substr(0,pos) == "Sigma" )
                    strval >> fVarInit;
                else if (line.substr(0,pos) == "SigmaMax" ){
                    strval >> fVarMax;
                    fVarMax *= fVarInit;
                }
                else if (line.substr(0,pos) == "SigmaMin" )
                    strval >> fVarMin;
                  else if (line.substr(0,pos) == "Alfa" )
                    strval >> fAlpha;
                else if (line.substr(0,pos) == "cf" ){
                    strval >> fCf;
                    backgroundRatio  = 1.0f - fCf;
                }
                else if (line.substr(0,pos) == "cT" )
                    strval >> fCT;
                else if (line.substr(0,pos) == "Range" )
                    strval >> varThreshold;
                else if (line.substr(0,pos) == "Gen" )
                    strval >> varThresholdGen;
                else if (line.substr(0,pos) == "Tau" )
                    strval >> fTau;
           
            
            }
        }
    }


}

string BackgroundSubtractorMOG3::initParametersToString()
{
    stringstream str;
    str << "--------------------------------------------------------------" << endl
        << "Default initialization parameters:"     << endl 
        << "GaussiansNo : " <<  nmixtures           << endl
        << "Sigma       : " << fVarInit             << endl
        << "SigmaMax    : " << fVarMax              << endl
        << "SigmaMin    : " << fVarMin              << endl
        << "Alpha       : " << fAlpha               << endl
        << "cf          : " << fCf                  << endl
        << "bgRation    : " << backgroundRatio      << endl
        << "cT          : " << fCT                  << endl
        << "Range       : " << varThreshold         << endl
        << "Gen         : " << varThresholdGen      << endl
        << "Tau         : " << fTau                 << endl
        << "------------------------------------------------------------" ;
    return str.str();
}

string BackgroundSubtractorMOG3::initParametersAsOneLineString()
{
    stringstream str;
    str << "# Alpha="      << fAlpha  
        << " cf="          << fCf 
        << " bgRation="    << backgroundRatio
        << " Range="       << varThreshold
        << " Gen="         << varThresholdGen
        << " GaussiansNo=" <<  nmixtures
        << " Sigma="       << fVarInit
        << " cT="          << fCT
        << " Tau="         << fTau;
    return str.str();
}

float BackgroundSubtractorMOG3::
globalIntensity(const Mat& _ref, const Mat& _current, string _method)
{

    return 0.0;
}


void BackgroundSubtractorMOG3::
saveModel()
{
    
    FileStorage fs("BackgroundModel.yml", FileStorage::WRITE);
    fs << "GaussianModel" << GaussianModel;
    fs << "CurrentGaussianModel" << CurrentGaussianModel;
    fs << "BackgroundNumberCounter" << BackgroundNumberCounter;
    fs << "Background" << Background ;
    fs << "Foreground" << Foreground;
    fs.release();
}

void BackgroundSubtractorMOG3::loadModel()
{
    FileStorage fs("BackgroundModel.yml", FileStorage::READ);
    fs["GaussianModel"] >> GaussianModel;
    fs["CurrentGaussianModel"] >> CurrentGaussianModel;
    fs["BackgroundNumberCounter"] >> BackgroundNumberCounter;
    fs["Background"] >> Background;
    fs["Foreground"] >> Foreground;
    alreadyInitialized = true;

}

void BackgroundSubtractorMOG3::setPointToDebug(const Point _pt) 
{ 
    pointToDebug.x = _pt.x;
    pointToDebug.y = _pt.y;
    
    cout << "Setting point: " << pointToDebug.x << " " << pointToDebug.y <<  endl;
};


/* End of file. */
