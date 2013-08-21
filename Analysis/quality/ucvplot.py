#!/usr/bin/python

from optparse import OptionParser
import sys
import re

import pylab as pl
from math import *

import numpy as np
from scipy.integrate import trapz

from rocplot import plotroc

   
    

        
class plotucv(plotroc) :

    def __init__(self, indexes=None, internal_range=None, one_value=None):
        #super(plotroc,self).__init__(indexes, internal_range, one_value)
        plotroc.__init__(self, indexes, internal_range, one_value)

    def plot1(self):
        self._set_title('MuHAVI-MAS ROC Curve')
        self._generic_plot(self.name_0, self.name_1, 'TPR_FPR1.png')
 
    def plot2(self):
        self._set_title('MuHAVI-MAS ROC Curve')
        self._generic_plot(self.name_1, self.name_0, 'TPR_FPR2.png')

    def plot3(self):
        self._set_title('MuHAVI-MAS ROC Curve\nRange plot')
        self._plot_range(self.name_0, self.name_1 , 'TPR_FPR3.png')

    def plot4(self):
        self._set_title('MuHAVI-MAS ROC Curve\nSingle plot')
        self._plot_one_line(self.name_0, self.name_1 , 'TPR_FPR4.png', self.parameter_value)






#############
        
if __name__ == '__main__':

    parser = OptionParser ()
    parser.add_option ("-f", "--file",dest = "file",
                       type = "string",
                       default = None,
                       help = "Select input file, e.g -f final_measures.txt")
    parser.add_option ("-c", "--colums",dest = "colums",
                       type = "string",
                       default = None,
                       help = "Select two colums to be processed, e.g -c 'Alpha,Threshold'")
    parser.add_option ("-r", "--range",dest = "range",
                       type = "string",
                       default = None,
                       help = "Plot selected range, e.g -r '0.01-0.03'")
    parser.add_option ("-v", "--value",dest = "value",
                       type = "string",
                       default = None,
                       help = "Plot single value, e.g -r '0.03'")
    (options, args) = parser.parse_args ()

    if options.file == None :
        parser.print_help()
        sys.exit()
 
    ##########
    #Variables
    #########

    n = plotucv(options.colums,options.range,options.value)
    n.load(options.file)
    n.plot1()
    n.plot2()
    n.plot3()
    n.plot4()
