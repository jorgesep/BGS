#!/usr/bin/python

from optparse import OptionParser
import sys
import os
import re

import pylab as pl
from math import *

import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import trapz
from frameplot import frameplot
from plot_single_roc import plotroc
  

colorList={'1':(0.98,0.01,0.74),\
           '2' : (1.0,0.60,0.00),\
           '3' : 'r',\
           '4' : (0.62,0.39,0.27),\
           '5' : (0.14,0.96,0.05),\
           '6' : 'g',\
           '7' : 'c',\
           '8' : (0.41,0.10,0.51),\
           '9' : 'b',\
           '10': (0.20,0.20,0.20)}

fontsizes = [4, 6, 8, 10, 16, 24, 32]

font = {'family' : 'serif',
        'color'  : 'darkred',
        'weight' : 'normal',
        'size'   : 16,
       }

# Activities
row_position = {'Person1' : 0 , 'Person4' : 1, 'Camera_3' : 0, 'Camera_4' : 1 }
ALGORITHMS   = ['mog2', 'sagmm']
ACTIONS      = ['Kick', 'Punch']
#ALGORITHMS   = ['mog2', 'sagmm', 'ucv_linear', 'ucv_staircase']
#ACTIONS      = ['Kick', 'Punch', 'RunStop' 'ShotGunCollapse' 'WalkTurnBack']
ACTORS       = ['Person1', 'Person4']
CAMERAS      = ['Camera_3', 'Camera_4']


def file_to_records():
    '''Read txt files and return a dictionary'''


def build_plot1(*args):
    _X   = args[0]
    _Y   = args[1]
    ax_1 = args[2]
    _seq = args[3]
    _act = args[4]
    _cam = args[5]
    ax_  = ax_1[row_position[_act]][row_position[_cam]]

    if row_position[_act] == 1:
        ax_.set_xlabel('False Positive Rate', fontsize=fontsizes[2] )
    if row_position[_cam] == 0:
        ax_.set_ylabel('True Positive Rate' , fontsize=fontsizes[2] )
    ax_.tick_params(axis='both', which='major', labelsize=8)
    ax_.grid()
    #plt.tight_layout()
    ax_.plot(_X, _Y, color='b', marker='x')
    ax_.text(0.99,0.02, _seq ,ha='right', va='bottom',fontsize=fontsizes[3],transform=ax_.transAxes)

def build_plot2(*args):
    _X   = args[0]
    _Y   = args[1]
    ax_  = args[2]
    _seq = args[3]
    _lP  = args[4]
    _lT  = args[5]
    _cnt = args[6]

    # Color counter
    p=int(fmod(_cnt,len(colorList)))
    if _cnt < 4 : 
        _lP.append(ax_.plot(_X, _Y, color=colorList[str(p+1)], ls='dashed', alpha=0.5)[0])
        ax_.plot(_X, _Y, color=colorList[str(p+1)],ls='dashed', alpha=0.5)
        label = '%s' % (_seq)
        _lT.append(label)
    else:
        _lP.append(ax_.plot(_X, _Y, color='b')[0])
        label = '%s Average' % (_seq)
        _lT.append(label)
        ax_.plot(_X, _Y,color='b', marker='x',lw=1.5)

        # Get first 12 (integers) range values.
        Z = [i for i,x in enumerate(_threshold) if (x%int(x))==0 and x <=10]
        for i in Z:
            ax_.text(_X[i]+0.0000, _Y[i]+0.01,_threshold[i],fontsize=8,color='black', horizontalalignment='center',verticalalignment='center')

        ax_.legend(_lP,_lT, bbox_to_anchor=(1,0), loc='lower right', prop={'size':8},numpoints=1, fancybox=True, shadow=True)
        ax_.tick_params(axis='both', which='major', labelsize=8)
        ax_.set_xlabel('False Positive Rate', fontsize=fontsizes[2] )
        ax_.set_ylabel('True Positive Rate', fontsize=fontsizes[2] )
        ax_.grid()


if __name__ == '__main__':

    parser = OptionParser ()
    parser.add_option ("-a", "--algorithm",dest = "alg",
                       type = "string",
                       default = None,
                       help = "Select algorithm name, e.g -a sagmm")
    parser.add_option ("-v", "--value",dest = "value",
                       type = "string",
                       default = None,
                       help = "Plot single value, e.g -r '0.03'")
    (options, args) = parser.parse_args ()

    if options.value == None:
        parser.print_help()
        sys.exit()

    learning_rate = options.value


    for alg in range(len(ALGORITHMS)):

        _algorithm = ALGORITHMS[alg] 

        # Create command of first figure
        cnt = 0

        # Initialize
        records  = None
        _actor   = None
        _camera  = None
        _tpr     = None
        _fpr     = None
        _threshold = None
        _action  = {} 
        _results = {} 

        #plt.figure(1)
        #fig1,  ax1= plt.subplots(2, 2, sharex=True, sharey=True)
        #fig1.subplots_adjust(hspace=0.001)
        # Define plots
        fig1,  ax1= plt.subplots(2, 2)
        fig1.set_tight_layout(True)
        fig2,  ax2= plt.subplots(1, 1)
        fig2.set_tight_layout(True)

        for i in range(len(ACTIONS)):


            _action = ACTIONS[i]

            # Counter for line colors
            legendP=[]
            legendT=[]

            # Loop through actors 'Person1' and 'Person4'.
            for j in range(len(ACTORS)):

                _actor = ACTORS[j]

                # Loop through all cameras 'Camera_3' and 'Camera_4'
                for k in range(len(CAMERAS)):

                    _camera = CAMERAS[k]

                    # Sequence name.
                    _sequence = ACTIONS[i] + '_' + ACTORS[j] + '_' + CAMERAS[k]
                    name     = _algorithm + '-' + _sequence
                    filename = _algorithm + '_' + _sequence + '.dat'

                    print filename

                    n = plotroc(one_value=learning_rate)
                    n.load(filename)
                    [X,Y,T,A] = n.array_of_values()

                    _array = np.array( [ X, Y, T ], dtype=float )

                    _results[name] = _array

                    if _threshold == None :
                        _threshold = np.array(T, dtype=float)

                    # Initialize empty array
                    if _tpr == None and _fpr == None :
                        _tpr = np.array([Y] , dtype=float)
                        _fpr = np.array([X] , dtype=float)
                    else:
                        _tpr = np.vstack((_tpr, np.array([Y] , dtype=float) ))
                        _fpr = np.vstack((_fpr, np.array([X] , dtype=float) ))

                    build_plot1(X, Y, ax1, _sequence, _actor, _camera)
                    build_plot2(X, Y, ax2, _sequence, legendP, legendT, cnt)
                    cnt +=1

            #
            # Average of results

            Y1 = _tpr.mean(axis=0)
            X1 = _fpr.mean(axis=0)
            build_plot2(X1, Y1, ax2, _action, legendP, legendT, cnt)

            fig1.savefig(_algorithm + '_' + ACTIONS[i] + '_1' )
            fig2.savefig(_algorithm + '_' + ACTIONS[i] + '_2' )

            plt.show()


        # Create plot
        #plt.clf()
        #fig, (ax1, ax2, ax3, ax4 ) = pl.subplots(nrows=2, ncols=2)


        #for key in _results.keys():
        #    if key.find(ACTIONS[i]) >=0:
        #        threshold = _array[2]
            

    #print _results
    #print _fpr
    #print _tpr

    #if options.range == None:
    #    n.plot6()
    #else:
    #    n.plot8()
    ##n.plot7()
    #n.plot9()

