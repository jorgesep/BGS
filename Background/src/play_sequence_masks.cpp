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

#include <stdio.h>
#include <opencv2/opencv.hpp>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sstream>
#include <map>

//#include "bgs.h"
//#include "mdgkt_filter.h"
//#include "background_subtraction.h"
//#include "Performance.h"
#include "utils.h"
#include "FrameReaderFactory.h"


#include "icdm_model.h"

using namespace cv;
using namespace std;
using namespace bgs;
using namespace seq;


const char* keys =
{
    "{ i | input   |       | Input directory of foreground files }"
    "{ n | ngrid   | 4     | Number of masks in grid of pictures }"
    "{ d | dir     | 0     | First directory of the grid  }"
    "{ t | threshold | false | Invert color of images.  }"
    "{ h | help    | false | Print help message }"
};


void display_message()
{
    cout << "Play foreground masks in special grid.                      " << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "                                                            " << endl;
    cout << "OpenCV Version : "  << CV_VERSION << endl;
    cout << "Example:                                                    " << endl;
    cout << "./play_sequence -i directory                                " << endl << endl;
    cout << "------------------------------------------------------------" << endl <<endl;
}






int main( int argc, char** argv )
{

    //declaration of local variables.
    stringstream msg,ptmsg;
    ofstream outfile, ptfile;

    //Parse console parameters
    CommandLineParser cmd(argc, argv, keys);


    // Read input parameters
    const string inputName   = cmd.get<string>("input");
    const int gridSize       = cmd.get<int>("ngrid");
    const int initDir        = cmd.get<int>("dir");
    const int inverted_color = cmd.get<bool>("threshold");

   cout << "GRID SIZE " << gridSize << endl;  

    if (cmd.get<bool>("help")) {
       display_message();
       cmd.printParams();
       return 0;
    }

    /// Initialization invert thresholding
    int threshold_value  = 200;
    int max_BINARY_value = 255;
    int threshold_type   = 1;


    /// aspect ratio of pictures 5:4
    int w = 200;
    int h = int(w/1.25);
    int border = 1;
    int sep    = 1;
    int ncol;
    int nrow;
    int wsize;
    int hsize;
    int m = border;
    int n = border;

    /// Prepare size of the grid: 2x2, 3x3, 4x4, 5x5, 6x6.
    if (gridSize < 5)  // 2X2
        ncol  = 2;

    else if (gridSize > 4 and gridSize < 10)  //3x3
        ncol  = 3;

    else if (gridSize > 9 and gridSize < 17)  //4x4
        ncol  = 4;

    else if (gridSize > 16 and gridSize < 26) { //5x5

        ncol  = 5;
        if ((ncol*w) > 600) w = 600/ncol;
        //w     = 320;
        h     = int(w/1.25);

    }
    else { // 6x6

        ncol  = 6;
        if ((ncol*w) > 600) w = 600/ncol;
        //w     = 266;
        h     = int(w/1.25);
    }

    nrow  = gridSize/ncol;
    nrow += (gridSize % ncol)? 1 : 0;

    wsize = border*2 + ncol*w + (ncol-1)*sep;
    hsize = border*2 + nrow*h + (nrow-1)*sep;


    /// Declare Map which will contain foreground masks directories.
    map<int, map<int,string> > ForegroundMapList;


    /// Populates the map with directory and list of foreground mask.
    MapAllMasksDirectories(inputName,ForegroundMapList);


    /// Create a window to display results
    string window_name ("Foreground Mask");
    string trackbar_value ("Value");

    namedWindow( window_name, CV_WINDOW_NORMAL  | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED );
    moveWindow ( window_name,50,50);
    //displayStatusBar(window_name, "Test", 0);


    /// Number of init and end mask of first directory, 
    /// it assumes other directories are similar.
    map<int, map<int,string> >::iterator initMap = ForegroundMapList.find(initDir);
    map<int, map<int,string> >::iterator itMap = initMap;
    int initFGMaskFrame = itMap->second.begin()->first;
    int endFGMaskFrame  = initFGMaskFrame + itMap->second.size();


    /// Main loop.
    for (int maskNumber = initFGMaskFrame; maskNumber < endFGMaskFrame; maskNumber++) {

        int cnt = 0;
        m = border;
        n = border;

        Mat mask;
        mask.create(Size(wsize, hsize),CV_8U);
        mask = Scalar::all(255);
        if (inverted_color)
            mask = Scalar::all(0);

        Mat img;
        Mat inv;


        // Loop for directories.
        for (itMap = initMap; itMap != ForegroundMapList.end(); ++itMap) {

            if ( itMap->first >= initDir ) {
                
                //map<int,string>::iterator it = itMap->second.begin();
                map<int,string>::iterator it = itMap->second.find(maskNumber);
                img = imread(it->second, CV_LOAD_IMAGE_GRAYSCALE);

                int x = img.cols;
                int y = img.rows;

                //int max = (x > y)? x: y;
                float wscale = (float) ( (float) x / w );
                float hscale = (float) ( (float) y / h );


                Rect roi( m, n, (int)( x/wscale ), (int)( y/hscale ) );

                Mat ImageROI = mask(roi);
                resize(img, ImageROI, ImageROI.size(), 0, 0);

                cnt++;

                /// Position in the grid.
                if ( (cnt%ncol) == 0) {
                    n += sep + h;
                    m  = border;
                }
                else
                    m += (sep + w);

                if ((cnt+1) > gridSize ) break;

            }
        }


        if (inverted_color) {
            threshold( mask, inv, threshold_value, max_BINARY_value,threshold_type );
            imshow(window_name, inv);
        }
        else 
            imshow(window_name, mask);


        /// Window interaction.
        int delay = 10;

        char key=0;
        key = (char)waitKey(delay);
        // Exit program
        if( key == 27 )
            break;
        
        // pause program
        if ( key == 32) {
            bool pause = true;
            while (pause) {

                key = (char)waitKey(delay);
                if (key == 32)
                    pause = false;
                // save frame
                if (key == 13) {
                    stringstream str;
                    str << maskNumber << ".png" ;
                    if (inverted_color)
                        imwrite( str.str()  , inv  );
                    else
                        imwrite( str.str()  , mask );
                    cout << "Saved image " << str.str() << endl;
                }
            }
        }


    }

    return 0;

}

