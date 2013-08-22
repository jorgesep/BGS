#!/usr/bin/python

from optparse import OptionParser
import sys
import re
import os

import pylab as pl
from math import *

import numpy as np
from scipy.integrate import trapz

from rocplot import plotroc

   
    

        
class plotucv(plotroc) :

    def __init__(self, indexes=None, internal_range=None, one_value=None, title=None, label=None):
        plotroc.__init__(self, indexes, internal_range, one_value)

        self.title = title
        if title == None:
            self.title = 'ROC Curve iGMM Staircase'

        self.label = label
        if label == None:
            self.label = 'MuHAVI-MAS WalkTurnBack Person1 Camera3'
        self._set_title(self.title)
        self._set_text (self.label)

    def plot1(self):
        self._generic_plot(self.name_0, self.name_1, 'TPR_FPR1.png')
 
    def plot2(self):
        self._generic_plot(self.name_1, self.name_0, 'TPR_FPR2.png')

    def plot3(self):
        self._plot_range(self.name_0, self.name_1 , 'TPR_FPR3.png')

    def plot4(self):
        self._plot_one_line(self.name_0, self.name_1 , 'TPR_FPR4.png', self.parameter_value)

    def plot5(self):
        self._plot_mean_and_median('TPR_FPR5.png', self.parameter_value)

    def plot6(self):
        self._plot_mean_and_median_same_graph('TPR_FPR6.png', self.parameter_value)







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
                       help = "Plot selected range, e.g -r '0.01-0.03' or -r '0.01 0.03 0.06' ")
    parser.add_option ("-v", "--value",dest = "value",
                       type = "string",
                       default = None,
                       help = "Plot single value, e.g -v '0.03'")
    parser.add_option ("-t", "--title",dest = "title",
                       type = "string",
                       default = None,
                       help = "Set title of the plot")
    parser.add_option ("-l", "--label",dest = "label",
                       type = "string",
                       default = None,
                       help = "Set bottom label text in the plot")
    parser.add_option ("-m", "--method",dest = "method",
                       type = "int",
                       default = 1,
                       help = "Set title of method, 1:Linear, 2:Staircase, 3:GMM normal")
    (options, args) = parser.parse_args ()

    if options.file == None :
        parser.print_help()
        sys.exit()
 
    ##########
    #Variables
    #########
    # Setting title for plots
    if options.title == None:
        if os.path.exists("ucv_labels.txt"):
            f = open('ucv_labels.txt', 'r')
            lines = f.readlines()
            f.close()
            title = [ i for i in lines if int(i[0]) == options.method ][0].split(',')[1]
            text  = [ i for i in lines if int(i[0]) == options.method ][0].split(',')[2].split('\n')[0]
    else:
        title = options.title
        text  = options.label


    n = plotucv(options.colums,options.range,options.value, title, text)
    n.load(options.file)
    n.plot1()
    n.plot2()
    n.plot3()
    n.plot4()
    n.plot5()
    n.plot6()
