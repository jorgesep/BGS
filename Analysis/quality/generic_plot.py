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
from frameplot import frameplot
   
    

        
class plot(plotroc) :

    def __init__(self, indexes=None, internal_range=None, one_value=None, title=None, label=None):
        plotroc.__init__(self, indexes, internal_range, one_value)

        self.title = title
        if title == None:
            self.title = 'ROC Curve <Algorithm Name>'

        self.label = label
        if label == None:
            self.label = 'MuHAVI-MAS'
        self._set_title(self.title)
        self._set_text (self.label)


    def plot1(self):
        self._generic_plot(self.name_0, self.name_1, 'GENERIC_TPR_FPR1.png')
 
    def plot2(self):
        self._generic_plot(self.name_1, self.name_0, 'GENERIC_TPR_FPR2.png')

    def plot3(self):
        self._plot_range(self.name_0, self.name_1 , 'GENERIC_TPR_FPR3.png')

    def plot4(self):
        self._plot_one_line(self.name_0, self.name_1 , 'GENERIC_TPR_FPR4.png', self.parameter_value)

    def plot5(self):
        self._plot_mean_and_median('GENERIC_TPR_FPR5.png', self.parameter_value)

    def plot6(self):
        self._plot_mean_and_median_same_graph('GENERIC_TPR_FPR6.png', self.parameter_value)

    def plot7(self):
        self._plot_mean_and_median_same_graph('GENERIC_TPR_FPR6.png', self.parameter_value)


class frame(frameplot):

    def __init__(self, indexes=None, internal_range=None, one_value=None, title=None, label=None):
        frameplot.__init__(self, indexes, internal_range, one_value)

        self.title = title
        if title == None:
            self.title = 'Performance Curve <Algorithm Name>'

        self.label = label
        if label == None:
            self.label = 'MuHAVI-MAS'
        self._set_title(self.title)
        self._set_text (self.label)


    def plot1(self):
        self._generic_plot(self.name_0, self.name_1, 'FRAME_TPR_FPR1.png')

    def plot2(self):
        self._plot_range('FRAME_TPR_FPR2.png')

    def plot3(self):
        self._plot_one_line('FRAME_TPR_FPR3.png')

    def plot4(self):
        self._generic_plot(self.name_0, self.name_1, 'FRAME_TPR_FPR4.png', 'MSSIM')

    def plot5(self):
        self._plot_range('FRAME_TPR_FPR5.png', None, 'MSSIM')


    def plot6(self):
        self._plot_one_line('FRAME_TPR_FPR6.png', None, 'MSSIM')


    def plot7(self):
        self._generic_plot(self.name_0, self.name_1, 'FRAME_TPR_FPR7.png', 'PSNR')

    def plot8(self):
        self._plot_range('FRAME_TPR_FPR8.png', None, 'PSNR')


    def plot9(self):
        self._plot_one_line('FRAME_TPR_FPR9.png', None, 'PSNR')

    def plot10(self):
        self._generic_plot(self.name_0, self.name_1, 'FRAME_TPR_FPR10.png', 'FMEASURE_MEAN')

    def plot11(self):
        self._plot_range('FRAME_TPR_FPR11.png', None, 'FMEASURE_MEAN')


    def plot12(self):
        self._plot_one_line('FRAME_TPR_FPR12.png', None, 'FMEASURE_MEAN')









def update_file(filename, col,val):
    f     = open(filename, 'r')
    lines = f.readlines()
    f.close()

    values = [i.strip() for i in lines[0].split(';')]
    values[col] = str(val)

    f     = open("generic_labels.txt", 'w')
    line="; ".join(values)
    print >> f,line
    f.close()

 

#############
        
if __name__ == '__main__':

    parser = OptionParser ()
    parser.add_option ("-f", "--file",dest = "file",
                       type = "string",
                       default = None,
                       help = "Select input file, e.g -f final_measures.txt")
    parser.add_option ("-c", "--columns",dest = "columns",
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
    parser.add_option ("-s", "--sequence_name",dest = "sequence_name",
                       type = "string",
                       default = None,
                       help = "Set name of sequence to processed")

    (options, args) = parser.parse_args ()

    if options.file == None :
        parser.print_help()
        sys.exit()
 
    ##########
    #Variables
    #########

    val   = options.value
    cols   = options.columns
    r4nge   = options.range
    title = options.title
    text  = options.label

    # Setting title for plots
    if options.title == None:
        if os.path.exists("generic_labels.txt"):
            f = open('generic_labels.txt', 'r')
            lines = f.readlines()
            f.close()

            title = [i.strip() for i in lines[0].split(';')][1]
            text  = [i.strip() for i in lines[0].split(';')][2]
            cols  = [i.strip() for i in lines[0].split(';')][3]
            if cols == "" :  cols = None
            r4nge = [i.strip() for i in lines[0].split(';')][4]
            if r4nge == "" :  r4nge = None
            val   = [i.strip() for i in lines[0].split(';')][5]
            if val == "" :  val   = None

        else :
            # Creating file
            title = "ROC Curve <Algorithm Name>"
            text  = "MuHAVI-MAS"
            seq   = "<Sequence Name>"
            cols  = 'LearningRate,Threshold'
            r4nge = '0.001-0.003'
            val   = '0.001'
            f     = open("generic_labels.txt", 'w')
            line="1;%s ;%s %s ; %s ; %s ; %s" % (title,text, seq, cols, r4nge, val)
            print >> f,line
            f.close()


   
    if options.value != None :
        # Save them in file
        val   = options.value
        update_file('generic_labels.txt',5,val)

    if options.range != None :
        # Save them in file
        r4nge   = options.range
        update_file('generic_labels.txt',4,r4nge)

    if options.columns != None :
        # Save them in file
        cols   = options.columns
        update_file('generic_labels.txt',3,cols)


    print "Plot ROC parameters\n================================="
    n = plot(cols,r4nge, val, title, text)
    n.load(options.file)
    n.plot1()
    n.plot2()
    n.plot3()
    n.plot4()
    n.plot5()
    n.plot6()

    print "\nPlot Performance parameters\n================================="
    title = "Performance <Algorithm Name>"
    m = frame(cols,r4nge, val, title, text)
    m.load(options.file)
    m.plot1()
    m.plot2()
    m.plot3()
    m.plot4()
    m.plot5()
    m.plot6()
    m.plot7()
    m.plot8()
    m.plot9()
    m.plot10()
    m.plot11()
    m.plot12()

