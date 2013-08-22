#!/usr/bin/python

from optparse import OptionParser
import sys
import re

import pylab as pl
from math import *

import numpy as np
from scipy.integrate import trapz


class plotroc :

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

        # Prepare string with parameters name and value of list
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

        # Define variables of main columns
        self.tp_idx  = self.parameters['TPR_MEAN']
        self.fp_idx  = self.parameters['FPR_MEAN']
        self.tpm_idx = self.parameters['TPR_MEDIAN']
        self.fpm_idx = self.parameters['FPR_MEDIAN']
        self.p0_idx  = self.parameters[self.name_0]
        self.p1_idx  = self.parameters[self.name_1]

        self._get_range()
        print "Plot range of '%s' %s%s%s" % (self.name_0,self.str_colors['blue'], " ".join([str(i) for i in self.range_values]), self.str_colors['reset'])
        print "Plot single line of '%s' %s%s%s" % (self.name_0, self.str_colors['red'], float(self.parameter_value), self.str_colors['reset'])


    def _set_title(self,title=None):
        self.title = 'MuHAVI-MAS ROC Curve'
        if title != None:
            self.title = title

    def _set_text(self,text=None):
        if text != None:
            self.text = text


    def _get_range(self, internal_range=None):

        # Assign parameters
        index_0 = self.parameters[self.name_0]
        index_1 = self.parameters[self.name_1]

       
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

    def _get_X_Y(self,col_0, col_1 ,key_val, tpr_0=None, fpr_0=None):
        '''col_1: is name of the parametric curve
           col_2: is variable which goes in the curve
           key_val: One value in col_1 
        '''

        # Assigns parameters
        index_0 = self.parameters[col_0]
        index_1 = self.parameters[col_1]

        # True positive rate and False positive rate
        if tpr_0 == None and fpr_0 == None:
            tpr    = self.tp_idx
            fpr    = self.fp_idx
        else :
            tpr = tpr_0
            fpr = fpr_0


        # get subset of key data, key is the number of parametric curve.
        _data = [p0 for p0 in self.data if float(p0[index_0]) == float(key_val)]

        # get dictionary of same data taking 'index2' as key
        _thre = {float(p0[index_1]):p0 for p0 in _data}

        values = _thre.keys()
        values.sort()

        # create a parameters list in ordered ascendent by index2 (col2)
        X = [float(_thre[val][fpr])    for val in values]
        Y = [float(_thre[val][tpr])    for val in values]
        A = [float(_thre[val][index_0]) for val in values]
        T = [float(_thre[val][index_1]) for val in values]

        auc = fabs(np.trapz(Y, X))

        return [X,Y,T,A]
         
            


    def _plot_mean_and_median(self,filename, val=None):
        # Assign parameters
        col_0 = self.name_0
        col_1 = self.name_1
        index_0 = self.parameters[col_0]
        index_1 = self.parameters[col_1]

        # True positive rate and False positive rate
        tpa    = self.tp_idx
        fpa    = self.fp_idx
        tpm    = self.tpm_idx
        fpm    = self.fpm_idx

        # Determine line to be plot, if not value takes middle
        value = self.parameter_value
        if value == None and val == None:
            value = self._get_middle_value()
        else:
            value = val


        # get mean values of parametric line.
        [X,Y,T,A]     = self._get_X_Y(col_0, col_1 ,value)
        if X == []: return

        # get median values of parametric line.
        [Xm,Ym,Tm,Am] = self._get_X_Y(col_0, col_1 ,value, tpm, fpm)
        if X == []: return

        # Create plot
        pl.clf()
        fig, (ax1, ax2) = pl.subplots(nrows=2, ncols=1)

        # Counter for line colors
        i=0
        legendP=[]
        legendT=[]

        # Color counter
        p=int(fmod(i,len(self.colorList)))

        # preparation of legend box in plot, e.g. 'LearningRate=0.001'
        label = '%s=%s' % ( str(self.parameters.keys()[self.parameters.values().index(index_0)]), str(value) )
        legendT.append(label)

        # Save object plot in legendP
        legendP.append(ax1.plot(X,Y,color=self.colorList[str(p+1)])[0])

        ax1.text(0.99, 0.05, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax1.transAxes, fontsize=self.fontsizes[2] )

        ax1.set_ylabel('True Positive Rate',  fontsize=self.fontsizes[2] )
        ax1.set_title( 'Mean ' + self.title , fontsize=self.fontsizes[3] )
        ax1.grid()

        ax1.plot(X,Y,color=self.colorList[str(p+1)],marker='x')
        ax1.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':6} , numpoints=1)
        ax1.tick_params(axis='both', which='major', labelsize=10)

        # Second plot
        legendP = []
        legendT = []
        p=int(fmod(i+1,len(self.colorList)))

        # preparation of legend box in plot.
        label = '%s=%s' % ( str(self.parameters.keys()[self.parameters.values().index(index_0)]), str(value) )
        legendT.append(label)

        ax2.text(0.99, 0.05, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax2.transAxes, fontsize=self.fontsizes[2] )
        ax2.set_ylabel('True Positive Rate',    fontsize=self.fontsizes[2] )
        ax2.set_xlabel('False Positive Rate',   fontsize=self.fontsizes[2] )
        ax2.set_title( 'Median ' + self.title , fontsize=self.fontsizes[3] )
        ax2.grid()

        legendP.append(ax2.plot(Xm,Ym,color=self.colorList[str(p+1)])[0])
        ax2.plot(Xm,Ym,color=self.colorList[str(p+1)],marker='x')
        ax2.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':6} , numpoints=1)

        #
        ax2.tick_params(axis='both', which='major', labelsize=10)
        #pl.tight_layout()
        fig.set_tight_layout(True)
        pl.savefig(filename)



    def _plot_mean_and_median_same_graph(self,filename, val=None):
        # Assign parameters
        col_0 = self.name_0
        col_1 = self.name_1
        index_0 = self.parameters[col_0]
        index_1 = self.parameters[col_1]

        # True positive rate and False positive rate
        tpa    = self.tp_idx
        fpa    = self.fp_idx
        tpm    = self.tpm_idx
        fpm    = self.fpm_idx

        # Determine line to be plot, if not value takes middle
        value = self.parameter_value
        if value == None and val == None:
            value = self._get_middle_value()
        else:
            value = val

        # get mean values of parametric line.
        [X,Y,T,A] = self._get_X_Y(col_0, col_1 ,value)
        if X == []: 
            return

        # Create plot
        pl.clf()
        fig, ax1 = pl.subplots(nrows=1, ncols=1)

        # Counter for line colors
        i=0
        legendP=[]
        legendT=[]

        # Color counter
        p=int(fmod(i,len(self.colorList)))

        # preparation of legend box in plot.
        legend = self.parameters.keys()[self.parameters.values().index(index_0)]
        label = '%s %s=%s' % ('Mean', str(legend),str(value))
        legendT.append(label)

        # Save object plot in legendP
        #pl.subplot(111)
        legendP.append(ax1.plot(X,Y,color=self.colorList[str(p+1)])[0])
        ax1.plot(X,Y,color=self.colorList[str(p+1)],marker='x')

        # get mean values of parametric line.
        [X,Y,T,A] = self._get_X_Y(col_0, col_1 ,value, tpm, fpm)
        if X == []: 
            return
        p=int(fmod(i+1,len(self.colorList)))
        # preparation of legend box in plot.
        legend = self.parameters.keys()[self.parameters.values().index(index_0)]
        label = '%s %s=%s' % ('Median', str(legend),str(value))
        legendT.append(label)

        # Save object plot in legendP
        legendP.append(pl.plot(X,Y,color=self.colorList[str(p+1)])[0])
        ax1.plot(X,Y,color=self.colorList[str(p+1)],marker='x')

        ax1.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56),  prop={'size':8},numpoints=1)
        
        ax1.tick_params(axis='both', which='major', labelsize=10)
        ax1.text(0.99, 0.05, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax1.transAxes, fontsize=self.fontsizes[2] )
        ax1.set_xlabel('False Positive Rate',  fontsize=self.fontsizes[2] )
        ax1.set_ylabel('True Positive Rate' , fontsize=self.fontsizes[2] )
        ax1.set_title(self.title, fontsize=self.fontsizes[3] )
        ax1.grid()
        #
        pl.savefig(filename)



    def _plot_one_line(self,col_0,col_1,filename, val=None):
        # Assign parameters
        index_0 = self.parameters[col_0]
        index_1 = self.parameters[col_1]

        # True positive rate and False positive rate
        tpr    = self.tp_idx
        fpr    = self.fp_idx


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
        [X,Y,T,A] = self._get_X_Y(col_0, col_1 ,value)
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
#===============
        Z = [ 0, int(len(T)/2), len(T)-1 ]
        for i in Z:        
            ax.text(X[i]+0.0000,Y[i]+0.002,T[i],fontsize=8,color='black', horizontalalignment='center',verticalalignment='center')
#
#        Z=X
#        Z.sort()
#        for i in range(len(Z)):
#            if Y[i] > 0.8 :
#                if A[i] == 0.001 :
#                    pl.text(X[i]+0.0000,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
#                else:
#                    pl.text(X[i]+0.00025,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
#            else:
#                pl.text(X[i]+0.00025,Y[i],T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
#
#
#
#
#
#
#
#===============
        ax.text(0.99, 0.02, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )
        ax.set_xlabel('False Positive Rate',  fontsize=self.fontsizes[2] )
        ax.set_ylabel('True Positive Rate',  fontsize=self.fontsizes[2] )
        ax.tick_params(axis='both', which='major', labelsize=self.fontsizes[2] )
        ax.set_title(self.title, fontsize=self.fontsizes[3] )
        ax.grid()
        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56),  prop={'size':8},numpoints=1)
        #
        pl.savefig(filename)


    def _plot_range(self,col_0,col_1,filename,prange=None):
        '''Plot a range of curves
           Range is a tuple or array
        '''
        # Assign parameters
        index_0 = self.parameters[col_0]
        index_1 = self.parameters[col_1]

        # True positive rate and False positive rate
        tpr    = self.tp_idx
        fpr    = self.fp_idx

        # Determine range
        #lookup_range = self.parameter_range
        #if prange != None:
        #    lookup_range = prange

        #if lookup_range != None:    
        #    sep = re.search('[0-9]+\.?[0-9]*(.)[0-9]+\.?[0-9]*',lookup_range).groups()[0]
        #    
        #    self.range_0 = ( float( lookup_range.split(sep)[0] ), float( lookup_range.split(sep)[1] ) )
        #else : 
        #    temp_line_range = { p0[index_0]:p0[index_0] for p0 in self.data }


        #    temp_values     = [float(val) for val in temp_line_range.keys()]
        #    temp_values.sort()
        #    temp_line_range=temp_values
        #    # 0.25 < range < 0.75
        #    self.range_0 = (float(temp_line_range[int(len(temp_line_range)*0.3)]),float(temp_line_range[int(len(temp_line_range)*0.7)]))

        ## Get range of col_0
        #r_0 = self.range_0[0]
        #r_1 = self.range_0[1]

        ## switch over 
        #if r_1 < r_0 :
        #    r_3 = r_0
        #    r_0 = r_1
        #    r_1 = r_3


        ## get all values for fixed parameter 
        #lines = {p0[index_0]:p0[index_0] for p0 in self.data if float(p0[index_0])>=r_0 and float(p0[index_0]) <=r_1}
        #values = [float(val) for val in lines.keys()]
        #values.sort()
        #lines=values

        self._get_range(prange)
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
            [X,Y,T,A] = self._get_X_Y(col_0, col_1 ,key)

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
        ax.set_xlabel('False Positive Rate', fontsize=self.fontsizes[2] )
        ax.set_ylabel('True Positive Rate', fontsize=self.fontsizes[2] )
        ax.set_title(self.title, fontsize=self.fontsizes[3] )
        ax.grid()
        pl.savefig(filename)

    def _generic_plot(self,col1,col2,filename):
        '''col1: is name of the parametric curve
           col2: is variable parameter in the curve
           filename is the name of png file
        '''

        # Assigns parameters
        index1 = self.parameters[col1]
        index2 = self.parameters[col2]

        # True positive rate and False positive rate
        tpr    = self.tp_idx
        fpr    = self.fp_idx

        # Create plot
        pl.clf()
        fig, ax = pl.subplots(nrows=1, ncols=1)

        # get all values for fixed parameter 
        lines = {p0[index1]:p0[index1] for p0 in self.data}
        values = [float(val) for val in lines.keys()]
        values.sort()
        lines=values

        # print name and value of fixed parameter
        print "%s: %s" %(str(col1), " ".join([str(i) for i in lines]))  

        # Counter for line colors
        i=0
        self.legendP=[]
        self.legendT=[]

        # get one line per time, arranged by index1.
        for key in lines :

            #print "key: " + str(index1) + " " + str(key)

            # get subset of key data, key is the number of parametric curve.
            _data = [p0 for p0 in self.data if float(p0[index1]) == key]

            # get dictionary of same data taking 'index2' as key
            _thre = {float(p0[index2]):p0 for p0 in _data}



            values = _thre.keys()
            values.sort()

            # create a parameters list in ordered ascendent by index2 (col2)
            X = [float(_thre[val][fpr])    for val in values]
            Y = [float(_thre[val][tpr])    for val in values]
            T = [float(_thre[val][index2]) for val in values]
            A = [float(_thre[val][index1]) for val in values]

            auc = fabs(np.trapz(Y, X))
         
            

            p=int(fmod(i,len(self.colorList)))


            # Save object plot in legendP
            self.legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legend = self.parameters.keys()[self.parameters.values().index(index1)]
            label = '%s=%s %.3f' % (str(legend),str(key),auc)
            #self.legendT.append(str(legend) + '=' + str(key) + ' ' + str(auc))
            self.legendT.append(label)

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='x')
 
            #print legend + ': ' + str(auc) 

            #for i in range(len(X)):
            #    if Y[i] > 0.8 :
            #        if A[i] == 0.001 :
            #            pl.text(X[i]+0.0000,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
            #        else:
            #            pl.text(X[i]+0.00025,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
            #    else:
            #        pl.text(X[i]+0.00025,Y[i],T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')

            i+=1
            #return
        #
        ax.legend(self.legendP,self.legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)
        ax.text(0.99, 0.05, self.text ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )
        ax.set_xlabel('False Positive Rate', fontsize=self.fontsizes[2] )
        ax.set_ylabel('True Positive Rate', fontsize=self.fontsizes[2] )
        ax.set_title(self.title, fontsize=self.fontsizes[3] )
        ax.grid()
        pl.savefig(filename)
#########################



#########################
         

    def plot1(self):
        '''Plot mean'''
        pl.clf()
        pl.subplot(111)

        tpr_index = self.parameters['TPR_MEAN']
        fpr_index = self.parameters['FPR_MEAN']
        self.tp_idx  = self.parameters['TPR_MEAN']
        self.fp_idx  = self.parameters['FPR_MEAN']
        self.tpm_idx = self.parameters['TPR_MEDIAN']
        self.fpm_idx = self.parameters['FPR_MEDIAN']

        print tpr_index
        print fpr_index

        X=[]
        Y=[]

        for p0 in self.data :
            #print p0
            X.append(float(p0[self.fp_idx]))
            Y.append(float(p0[self.tp_idx]))
        
        #
        pl.plot(X,Y,'b-',lw=1)
        pl.title('Non Parametric\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR.png')

    def plot2(self):
        '''Threshold fixed alpha variable'''

        pl.clf()
        pl.subplot(111)

        tpr_index = self.parameters['TPR_MEAN']
        fpr_index = self.parameters['FPR_MEAN']
        threshold = self.parameters['Threshold']
        alpha     = self.parameters['Alpha']

        #print threshold

        param = {p0[threshold]:p0[threshold] for p0 in self.data}
        values= [float(val) for val in param]
 
        values.sort()
        param=values
        print param

        i=0
        legendP=[]
        legendT=[]

        for t in param :

            X=[float(p0[fpr_index]) for p0 in self.data if float(p0[threshold]) == t]
            Y=[float(p0[tpr_index]) for p0 in self.data if float(p0[threshold]) == t]
            T=[float(p0[threshold]) for p0 in self.data if float(p0[threshold]) == t]
            A=[float(p0[alpha]) for p0 in self.data if float(p0[threshold]) == t]
           
            p=int(fmod(i,len(self.colorList)))
            print p
            legendP.append(pl.plot(X,Y,color=self.colorList[str(p+1)])[0])
            legendT.append('Threshold='+str(t))
            #if r == 30 :
            pl.plot(X,Y,color=self.colorList[str(p+1)],marker='x')                
 
            i+=1
        #
        pl.legend(legendP,legendT, bbox_to_anchor=(0.95,0.76), fontsize=6,numpoints=1)
        pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR2.png')
        
    def plot3(self):
        '''Alpha fixed and threshold variable'''

        index     = self.p1_idx
        threshold = self.p0_idx

        pl.clf()
        pl.subplot(111)

        params = {p0[index]:p0[index] for p0 in self.data}
        values= [float(val) for val in params]
        values.sort()
        params=values

        i=0
        legendP=[]
        legendT=[]

        #ordered by alpha
        for key in params :

            # get subset of data indexed by 'key'. In this time 'key' is alpha 
            _data = [p0 for p0 in self.data if float(p0[index]) == key]

            # get dictionary of same data taking 'threshold' as key
            _thre = {float(p0[threshold]):p0 for p0 in _data}
            values = _thre.keys()
            values.sort()

            # create a list parameters in ascendent order by threshold
            X = [float(_thre[val][self.fp_idx]) for val in values]
            Y = [float(_thre[val][self.tp_idx]) for val in values]
            T = [float(_thre[val][self.p0_idx]) for val in values]
            A = [float(_thre[val][self.p1_idx]) for val in values]
          
            p=int(fmod(i,len(self.colorList)))

            legendP.append(pl.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legend = self.parameters.keys()[self.parameters.values().index(index)]
            legendT.append(str(legend) + '=' + str(key))
            #legendT.append('Alpha='+str(key))
            #if r == 30 :
            pl.plot(X,Y,color=self.colorList[str(p+1)],marker='x')                
#####################################
            if key == 0.1 :
                pl.plot(X,Y,color=self.colorList[str(p+1)],marker='x')                
                label_counter = 0
                for i in range(len(X)):
                    if int(fmod(label_counter,5)) == 0:
                        number = '%.1e'% T[i]
                        pl.text(X[i],Y[i]+0.02,number,fontsize=8,color='black', horizontalalignment='center',verticalalignment='center')
                    label_counter += 1

                    #if Y[i] > 0.8 :
                    #    if A[i] == 0.001 :
                    #        pl.text(X[i]+0.0000,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                    #    else:
                    #        pl.text(X[i]+0.00025,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                    #else:
                    #    pl.text(X[i]+0.00025,Y[i],T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
#####################################
            i+=1
        #
        pl.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), fontsize=6,numpoints=1)
        pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR3.png')
        
    def plot4(self):

        self._generic_plot('Threshold','Alpha')
        #pl.legend(self.legendP,self.legendT, bbox_to_anchor=(0.99,0.56), numpoints=1)
        #pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        #pl.xlabel('False Positive Rate')
        #pl.ylabel('True Positive Rate')
        #pl.grid()
        ##
        #pl.savefig('NP_TPR-FPR4.png')
        
    def plot5(self): 
        '''MCC fixed alpha variable'''

        pl.clf()
        pl.subplot(111)

        mcc = self.parameters['MCC_MEAN']
        pnsr = self.parameters['PNSR_MEAN']
        threshold = self.parameters['Threshold']
        alpha     = self.parameters['Alpha']

        #print threshold

        param = {p0[threshold]:p0[threshold] for p0 in self.data}
        values= [float(val) for val in param]
 
        values.sort()
        param=values
        print param

        i=0
        legendP=[]
        legendT=[]

        for t in param :

            X=[float(p0[threshold]) for p0 in self.data if float(p0[threshold]) == t]
            Y=[float(p0[mcc]) for p0 in self.data if float(p0[threshold]) == t]
            T=[float(p0[threshold]) for p0 in self.data if float(p0[threshold]) == t]
            A=[float(p0[alpha]) for p0 in self.data if float(p0[threshold]) == t]
           
            p=int(fmod(i,len(self.colorList)))
            legendP.append(pl.plot(X,Y,color=self.colorList[str(p+1)])[0])
            legendT.append('Threshold='+str(t))
            #if r == 30 :
            pl.plot(X,Y,color=self.colorList[str(p+1)],marker='x')                
 
            i+=1
        #
        pl.legend(legendP,legendT, bbox_to_anchor=(0.95,0.76), fontsize=6,numpoints=1)
        pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR5.png')

    def plot6(self):
        self._set_title('MuHAVI-MAS ROC Curve')
        self._generic_plot(self.name_0, self.name_1, 'NP_TPR-FPR6.png')
 
    def plot7(self):
        self._set_title('MuHAVI-MAS ROC Curve')
        self._generic_plot(self.name_1, self.name_0, 'NP_TPR-FPR7.png')

    def plot8(self):
        self._set_title('MuHAVI-MAS ROC Curve\nRange plot')
        self._plot_range(self.name_0, self.name_1 , 'NP_TPR_FPR8.png')

    def plot9(self):
        self._set_title('MuHAVI-MAS ROC Curve\nSingle plot')
        self._plot_one_line(self.name_0, self.name_1 , 'NP_TPR_FPR9.png', self.parameter_value)


class sepu :

    # ########################
    #
    def  __init__(self):
        # define parameters
        print 'sepu'
        
    # ######################
    #
    def load(self,name) :
        Fi=file(name,'r')  
        
        l=Fi.read().split('\n')
        self.data=[]
        for l0 in l :
            l1=l0.split()
            if len(l1) > 1 and l1[0] != '#':
                self.data.append(l1)
    # ######################
    #
    def plot1(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        
        X=[]
        Y=[]
        for p0 in self.data :
            #print p0
            X.append(float(p0[10]))
            Y.append(float(p0[9]))
        
        pl.plot(X,Y,'o')

            

        #
        pl.xlabel('TPR')
        pl.ylabel('FPR')
        #
        pl.savefig('TPR-FPR.png')


    # ######################
    #
    def plot2(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        
        X=[]
        Y=[]
        for p0 in self.data :
            
            if float(p0[9]) > 0.4 :
                X.append(float(p0[10]))
                Y.append(float(p0[9]))
        
                
        pl.plot(X,Y,'o')

        

        #
        pl.xlabel('FPR')
        pl.ylabel('TPR')
        #
        pl.savefig('TPR-FPR2.png')



    # ######################
    #
    def plot3(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        
        X=[]
        Y=[]
        R=[]
        for p0 in self.data :            
            if float(p0[9]) > 0.4 :
                X.append(float(p0[10]))
                Y.append(float(p0[9]))
                R.append(p0[3])
        
                
        pl.plot(X,Y,'o')
        
        for i in range(len(X)): 
            pl.text(X[i],Y[i]-0.006,R[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')

            
        #
        pl.xlabel('FPR')
        pl.ylabel('TPR')
        #
        pl.savefig('TPR-FPR3.png')


    # ######################
    #
    def plot4(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        


        rang={}
        for p0 in self.data :            
            rang[p0[3]]=p0[3]
        rang2=[]
        for r in rang:
            rang2.append(float(r))
        
        print rang2
        rang2.sort()
        rang=rang2
        print rang
        colorList={'1':(0.98,0.01,0.74),'3':'r','2':(1.0,0.60,0.00),'4':(0.62,0.39,0.27),'5':(0.14,0.96,0.05),'6':'g','7':'c','9':'b','8':(0.41,0.10,0.51),'10':(0.20,0.20,0.20)}
        i=0
        legendP=[]
        legendT=[]
        for r in rang :
            
            X=[]
            Y=[]
            R=[]
            A=[]
            
            for p0 in self.data :            
                if float(p0[3]) == r :
                    X.append(float(p0[10]))
                    Y.append(float(p0[9]))
                    R.append(float(p0[3]))
                    A.append(float(p0[0]))
            
            p=int(fmod(i,len(colorList)))
            print p
            legendP.append(pl.plot(X,Y,color=colorList[str(p+1)])[0])
            legendT.append('Range='+str(r))
            #if r == 30 :
            pl.plot(X,Y,color=colorList[str(p+1)],marker='x')                
            if r == 10 :
                pl.plot(X,Y,color=colorList[str(p+1)],marker='x')                
                for i in range(len(X)):
                    if Y[i] > 0.8 :
                        if A[i] == 0.001 :
                            pl.text(X[i]+0.0000,Y[i]+0.02,A[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                        else:
                            pl.text(X[i]+0.00025,Y[i]+0.02,A[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                    else:
                        pl.text(X[i]+0.00025,Y[i],A[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
            i+=1
        #
        pl.legend(legendP,legendT, bbox_to_anchor=(0.95,0.76), numpoints=1)
        pl.title('Self Adaptive Gaussian Mixture Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('TPR-FPR4.png')
        
    






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
    #s=sepu()

#  Alpha=1 cf=2 bgRation=3 Range=4 Ge=5n GaussiansNo=6 Sigma=7 cT=8 Tau=9 TPR=10 FPR=11 SPE MCC TPR TNR SPE MCC

    #s.load('final_measures.txt')
    #
    #s.plot1()
    #s.plot2()
    #s.plot3()
    #s.plot4()
                       
    n = plotroc(options.colums,options.range,options.value)
    #n.load('np_final_measures.txt')
    n.load(options.file)
    #n.area_under_curve()
    #n.plot1()
    #n.plot2()
    #n.plot3()
    #n.plot4()
    #n.plot5()
    #
    n.plot6()
    n.plot7()
    n.plot8()
    n.plot9()


