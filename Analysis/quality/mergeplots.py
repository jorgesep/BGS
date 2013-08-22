#!/usr/bin/python

from optparse import OptionParser
import sys
import re
import os

import pylab as plt
from math import *

import numpy as np
from scipy.integrate import trapz

from rocplot import plotroc

   
class loadfile:

    def __init__(self,files):
        self.files = files
        self.data   = {}
        self.params = {}
        self.colname= {}

    def load(self,filename):
        f=open(filename)
        one_line = f.readlines()
        f.close()

        lines=[i.rstrip() for i in lines]
        parameters = {lines[0].split()[i+1]:i for i in range(len(lines[0].split()[1:])) if lines[0].split()[0] == '#'}
        data       = [l0.split() for l0 in lines[1:] if len(l0.split()) > 1]

        self.params[filename] = parameters
        self.data[filename]   = data

        listp= [i for i in parameters.keys() if parameters[i] < parameters['TPR_MEAN'] ]
        listp.sort()
        self.colname[filename] = "".join([name+':'+ str(parameters[name])+' '  for name in listp])


        
class mergeplots :

    def __init__(self, files, algorithm, columns, values):

        self.colorList={'1':(0.98,0.01,0.74),\
                        '2' : (1.0,0.60,0.00),\
                        '3' : 'r',\
                        '4' : (0.62,0.39,0.27),\
                        '5' : (0.14,0.96,0.05),\
                        '6' : 'g',\
                        '7' : 'c',\
                        '8' : (0.41,0.10,0.51),\
                        '9' : 'b',\
                        '10': (0.20,0.20,0.20)}

        self.fontsizes = [4, 6, 8, 10, 16, 24, 32]

        self.files = files
        self.algorithm = algorithm
        self.columns = columns
        self.values = values
        self.plt = []
        self.set_title()

    def set_title(self, title=None, sequence=None):

        self.title = title
        if title == None:
            self.title = 'ROC Curve'

        self.sequence_name = sequence
        if sequence == None:
            self.sequence_name = 'MuHAVI-MAS'

    def load(self):
        for i in range(len(self.files)):
            np = plotroc(self.columns[i], one_value = self.values[i])
            print "FIles: " + self.files[i]
            np.load(self.files[i])
            self.plt.append(np)
            print "------------------------------------------------"

    def plot1(self):

        filename = 'roc_plot.png'
        # Create plot
        plt.clf()
        fig, ax = plt.subplots(nrows=1, ncols=1)

        # Counter for line colors
        i=0
        legendP=[]
        legendT=[]


        for i in range(len(self.files)):
            col_0 = self.columns[i].split(',')[0]
            col_1 = self.columns[i].split(',')[1]
            value = float(self.values[i])

            # get values of parametric line.
            [X,Y,T,A] = self.plt[i]._get_X_Y(str(col_0), str(col_1) ,value)

            # Color counter
            p=int(fmod(i,len(self.colorList)))

            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            label = '%s' % (str(self.algorithm[i]))
            legendT.append(label)
            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='x')
            i+=1


        #
        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)
        ax.text(0.99, 0.05, self.sequence_name ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )
        ax.set_xlabel('False Positive Rate', fontsize=self.fontsizes[2] )
        ax.set_ylabel('True Positive Rate', fontsize=self.fontsizes[2] )
        ax.set_title(self.title, fontsize=self.fontsizes[3] )
        ax.grid()
        plt.savefig(filename)



def update_file(filename,files,algorithm,sequence,columns,values) :
    # Save them in file
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()

    f = open(filename, 'w')
    print >> f,lines[0].strip() 

    for i in range(len( files )):
        line = "%s ; %s ; %s ; %s ; %s" % (files[i],algorithm[i],sequence[i],columns[i],values[i])
        print >> f,line
    f.close()





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
                       help = "Select two colums to be processed, e.g -c '0:Alpha,Threshold'")
    parser.add_option ("-v", "--values",dest = "values",
                       type = "string",
                       default = None,
                       help = "Plot using each value, e.g -v '0.03 0.005'")
    parser.add_option ("-t", "--title",dest = "title",
                       type = "string",
                       default = None,
                       help = "Set title of the plot")
    parser.add_option ("-l", "--label",dest = "label",
                       type = "string",
                       default = None,
                       help = "Set bottom label text in the plot")
    parser.add_option ("-p", "--print",dest = "print",
                       type = "string",
                       default = None,
                       help = "Print by console main parameters")
    (options, args) = parser.parse_args ()

    #if options.file == None :
    #    parser.print_help()
    #    sys.exit()
 
    ##########
    #Variables
    #########
    # Setting title for plots
    files = {}
    columns = {}
    values = {}
    lines = []
    if os.path.exists("files.txt"):
            f = open('files.txt', 'r')
            files = f.readlines()
            f.close()

            #remove first line
            lines = [ i for i in files if i[0] != '#' ]

            files     = {i:lines[i].split(';')[0].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            algorithm = {i:lines[i].split(';')[1].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            sequence  = {i:lines[i].split(';')[2].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            columns   = {i:lines[i].split(';')[3].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            values    = {i:lines[i].split(';')[4].strip() for i in range(len(lines)) if lines[i][0] != '#' }

    else:
        sys.exit()

    if options.values != None :
        # Update values from console
        for i in range(len( options.values.split() )):
            values[i] = options.values.split()[i]

        # Save them in file
        update_file('files.txt',files,algorithm,sequence,columns,values)

    if options.colums != None :
        # Update columns from console
        for i in range(len( options.columns.split() )):
            columns[i] = options.columns.split()[i]

        # Save them in file
        update_file('files.txt',files,algorithm,sequence,columns,values)


    n = mergeplots(files, algorithm, columns, values) 
    n.load()
    n.plot1()

