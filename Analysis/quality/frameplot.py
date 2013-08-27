#!/usr/bin/python

from optparse import OptionParser
import sys
import re

import pylab as pl
from math import *

import numpy as np
from scipy.integrate import trapz


class frameplot :

    def __init__(self, indexes=None, internal_range=None, one_value=None):
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


        self.str_colors = {'reset':'\033[0m', 'blue':'\033[34m', 'red':'\033[32m'}
        self.parameters_name = ''
        self.indexes = indexes
        self.parameter_range = internal_range
        self.parameter_value = one_value
        self.text = ''
        self.range_values = []
        self.range_0 = ()

    def print_parameter_values(self):
        print self.parameters_name

    def area_under_curve(self,X,Y):
        pass



    def load(self, name) :
        '''Load txt file to be processed.'''
        Fi = file(name,'r')

        lines = Fi.read().split('\n')

        # read first line of file to get main parameter values. 
        self.parameters = {lines[0].split()[i+1]:i for i in range(len(lines[0].split()[1:])) if lines[0].split()[0] == '#'}

        # keep all values in data array.
        self.data  = [l0.split() for l0 in lines[1:] if len(l0.split()) > 1]

        # Prepare string with variables of algorithm name and value of list
        listp = [key for key in self.parameters.keys() if not key.find("MEDIAN") > 0 and not key.find("MEAN") > 0]
        listp.sort()
        names = [name+':'+ str(self.parameters[name])+' '  for name in listp]
        self.parameters_name = ''.join(names)
        print self.parameters_name

        
        if self.indexes == None:
            # By default takes first two parameters
            _idx = [i for i in lines[0].split() if i != '#']
            self.name_0 = _idx[0]
            self.name_1 = _idx[1]
        else:
            # Takes parameters defined in indexes
            self.name_0 = self.indexes.split(',')[0]
            self.name_1 = self.indexes.split(',')[1]

        # Get number of columns starting in zero
        self.fm_idx  = self.parameters['FMEASURE_MEAN']
        self.mc_idx  = self.parameters['MCC_MEAN']
        self.ps_idx  = self.parameters['PSNR']
        self.ms_idx  = self.parameters['MSSIM']
        self.ds_idx  = self.parameters['DSCORE']
        self.p0_idx  = self.parameters[self.name_0]
        self.p1_idx  = self.parameters[self.name_1]

        self._get_range()
        #temp_line_p0 = { p0[self.p0_idx]:str(p0[self.p0_idx]) for p0 in self.data }
        #temp_values  = [float(val) for val in temp_line_p0.keys()]
        #temp_values.sort()
        #temp_line_p0 = { p0:"%s%s%s"%(self.str_colors['blue'], p0, self.str_colors['reset']) for p0 in self.range_values }
        #ran_str = ""
        #for i in range(len(temp_values)):
        #    if temp_line_p0.has_key(i):
        #        ran_str += str(temp_line_p0[i]) + " "
        #    else:
        #        ran_str += str(temp_values[i]) + " "
        # 
        #print ran_str 
        print "Plot range of '%s' %s%s%s" % (self.name_0,self.str_colors['blue'], " ".join([str(i) for i in self.range_values]), self.str_colors['reset'])
        print "Plot value of '%s' %s%s%s" % (self.name_0, self.str_colors['red'], float(self.parameter_value), self.str_colors['reset'])

# LearningRate cf bgRation Threshold Gen GaussiansNo Sigma cT Tau TPR_MEAN FPR_MEAN FMEASURE_MEAN MCC_MEAN TPR_MEDIAN FPR_MEDIAN FMEASURE_MEDIAN MCC_MEDIAN PSNR MSSIM DSCORE



    def _set_title(self,title=None):
        self.title = 'MuHAVI-MAS'
        if title != None:
            self.title = title

    def _set_text(self,text=None):
        if text != None:
            self.text = text


    def _get_range(self, internal_range=None):

        # Assign parameters
        index_0 = self.p0_idx
        index_1 = self.p1_idx
       
        # Determine range
        # Determine range from self.data
        if internal_range == None and self.parameter_range == None:

            temp_line_range = { p0[index_0]:p0[index_0] for p0 in self.data }
            temp_values     = [float(val) for val in temp_line_range.keys()]
            temp_values.sort()
            temp_line_range=temp_values
            # 0.25 < range < 0.75
            self.range_0 = (float(temp_line_range[int(len(temp_line_range)*0.3)]),float(temp_line_range[int(len(temp_line_range)*0.7)]))

        else:
            lookup_range = self.parameter_range
            if internal_range != None:
                lookup_range = internal_range
 
            # verify if the input is a range
            if lookup_range.find('-') > 0:
                sep = re.search('[0-9]+\.?[0-9]*(.)[0-9]+\.?[0-9]*',lookup_range).groups()[0]
                self.range_0 = ( float( lookup_range.split(sep)[0] ), float( lookup_range.split(sep)[1] ) )
            
            else :
                values = [ float(i) for i in lookup_range.split() ]
                values.sort()
                self.range_values = values

        # According previous result determine a range of values
        if len(self.range_0) == 2:
            # Get range of col_0
            r_0 = self.range_0[0]
            r_1 = self.range_0[1]
    
            # switch over 
            if r_1 < r_0 :
                r_3 = r_0
                r_0 = r_1
                r_1 = r_3

            # get all values for fixed parameter 
            lines = {p0[index_0]:p0[index_0] for p0 in self.data if float(p0[index_0])>=r_0 and float(p0[index_0]) <=r_1}
            values = [ float(val) for val in lines.keys() ]
            values.sort()
            self.range_values = values

        # Set one just one value to be plot
        if self.parameter_value == None:
            self.parameter_value = float( self.range_values[int(len(self.range_values)*0.5)] )



    def _get_middle_value(self):
        # takes middle
        middle_value = { p0[self.p0_idx]:p0[self.p0_idx] for p0 in self.data }
        temp_values     = [float(val) for val in middle_value.keys()]
        temp_values.sort()
        middle_value=temp_values
        value = float( middle_value[int(len(middle_value)*0.5)] )
        return value

    def _get_X_Y(self,key_val, value=None):
        '''col_1: is name of the parametric curve
           col_2: is variable which goes in the curve
           key_val: One value in col_1 
        '''

        # Assign parameters
        index_0 = self.p0_idx
        index_1 = self.p1_idx

        # Measure: MCC, FMEASURE, PSNR or DSCORE. By default MCC
        if value == None :
            value = self.mc_idx


        # get subset of key data, key is the number of parametric curve.
        _data = {float(p0[index_1]):float(p0[value]) for p0 in self.data if float(p0[index_0]) == float(key_val)}

        values = _data.keys()
        values.sort()

        # create a parameters list in ordered ascendent by index2 (col2)
        X = [float(val)        for val in values]
        Y = [float(_data[val]) for val in values]

        return [X,Y]
         
 
    def _plot_one_line(self,filename, val=None, param=None):
        # Assign parameters
        index_0 = self.p0_idx
        index_1 = self.p1_idx

        if param == None :
            parameter = self.mc_idx
        else :
            parameter = self.parameters[param]

        # Determine line to be plot
        value = self.parameter_value
        if val != None:
            value = val

        # if not value, takes middle
        if value == None:
            middle_value = { p0[index_0]:p0[index_0] for p0 in self.data }
            temp_values     = [float(val) for val in middle_value.keys()]
            temp_values.sort()
            middle_value=temp_values
            value = float( middle_value[int(len(middle_value)*0.5)] )

        # get values of parametric line.
        [X,Y] = self._get_X_Y(value, parameter)
        if X == []: return


        # Create plot
        pl.clf()
        fig, ax = pl.subplots(nrows=1, ncols=1)

        # Counter for line colors
        i=0
        legendP=[]
        legendT=[]

        # Color counter
        p=int(fmod(i,len(self.colorList)))


        # Save object plot in legendP
        legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

        # preparation of legend box in plot.
        legend = self.parameters.keys()[self.parameters.values().index(index_0)]
        label = '%s=%s' % (str(legend),str(value))
        legendT.append(label)

        ax.plot(X,Y,color=self.colorList[str(p+1)],marker='x')
        ax.text(0.99, 0.02, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )

        xlabel = self.parameters.keys()[self.parameters.values().index(index_1)]
        ax.set_xlabel(xlabel ,  fontsize=self.fontsizes[2] )

        ylabel = self.parameters.keys()[self.parameters.values().index(parameter)]
        ax.set_ylabel(ylabel,  fontsize=self.fontsizes[2] )

        ax.tick_params(axis='both', which='major', labelsize=self.fontsizes[2] )
        ax.set_title(self.title, fontsize=self.fontsizes[3] )
        ax.grid()
        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56),  prop={'size':8},numpoints=1)
        #
        pl.savefig(filename)



    def _plot_range(self, filename,_range=None, _param=None):
        '''Plot a range of curves
           Range is a tuple or array
        '''
        # Assign parameters
        index_0 = self.p0_idx
        index_1 = self.p1_idx

        # Verify parameter: Example MCC, DSCORE ...
        if _param == None :
            parameter = self.mc_idx
        else :
            parameter = self.parameters[_param]


        # Get a range of first param: Example LearningRate : [0.001, 0.002, 0.003]
        self._get_range(_range)
        lines = self.range_values

        if lines == [] :
            return

        #print "Plot range of '%s'%s%s%s" % (col_0,self.str_colors['blue'], " ".join([str(i) for i in lines]), self.str_colors['reset'])

        # Create plot
        pl.clf()
        fig, ax = pl.subplots(nrows=1, ncols=1)

        # Counter for line colors
        i=0
        legendP=[]
        legendT=[]

        # get one line per time, arranged by index1.
        for key in lines :

            # get values of parametric line.
            [X,Y] = self._get_X_Y(key, parameter)

            # Color counter
            p=int(fmod(i,len(self.colorList)))

            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legend = self.parameters.keys()[self.parameters.values().index(index_0)]
            label = '%s=%s' % (str(legend),str(key))
            legendT.append(label)

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='x')
            i+=1
 
        #
        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)
        ax.text(0.99, 0.05, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )
        xlabel = self.parameters.keys()[self.parameters.values().index(index_1)]
        ylabel = self.parameters.keys()[self.parameters.values().index(parameter)]

        ax.set_xlabel(xlabel, fontsize=self.fontsizes[2] )
        ax.set_ylabel(ylabel, fontsize=self.fontsizes[2] )
        ax.set_title(self.title, fontsize=self.fontsizes[3] )
        ax.grid()
        pl.savefig(filename)

    def _generic_plot(self, col0, col1, filename, param=None ):
        '''col1: is name of the parametric curve
           col2: is variable parameter in the curve
           filename is the name of png file
        '''
        print "PARAM: %s %s %s %s" % (self.name_0, self.name_1, filename, param)

        # Assigns parameters
        index_0 = self.parameters[col0]
        index_1 = self.parameters[col1]

        if param == None :
            parameter = self.mc_idx
        else :
            parameter = self.parameters[param]


        # Create plot
        pl.clf()
        fig, ax = pl.subplots(nrows=1, ncols=1)

        # get all values for fixed parameter 
        lines = {p0[index_0]:p0[index_0] for p0 in self.data}
        values = [float(val) for val in lines.keys()]
        values.sort()
        lines=values

        # print name and value of fixed parameter
        print "%s: %s" %(str(col0), " ".join([str(i) for i in lines]))  

        # Counter for line colors
        i=0
        legendP=[]
        legendT=[]

        # get one line per time, arranged by index1.
        for key in lines :

            #print "key: " + str(index1) + " " + str(key)

            # get subset of key data, key is the number of parametric curve.
            _data = [p0 for p0 in self.data if float(p0[index_0]) == key]

            # get dictionary of same data taking 'index2' as key
            _thre = {float(p0[index_1]):float(p0[parameter]) for p0 in _data}

            values = _thre.keys()
            values.sort()

            # create a parameters list in ordered ascendent by index2 (col2)
            X = [float(val)        for val in values]
            Y = [float(_thre[val]) for val in values]

            p=int(fmod(i,len(self.colorList)))


            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legend = self.parameters.keys()[self.parameters.values().index(index_0)]
            label = '%s=%s' % (str(legend),str(key))
            legendT.append(label)

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='x')

            i+=1
            #return
        #
        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)
        ax.text(0.99, 0.05, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )
        xlabel = self.parameters.keys()[self.parameters.values().index(index_1)]
        ylabel = self.parameters.keys()[self.parameters.values().index(parameter)]
        ax.set_xlabel( xlabel, fontsize=self.fontsizes[2] )
        ax.set_ylabel( ylabel, fontsize=self.fontsizes[2] )
        ax.set_title(self.title, fontsize=self.fontsizes[3] )
        ax.grid()
        pl.savefig(filename)
#########################


