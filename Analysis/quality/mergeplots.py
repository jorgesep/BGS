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
from frameplot import frameplot
   
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



class msplot :
    '''
    inputfile: each row separate by semicolon contains the name of file, algorithm, sequence, main parameters and fixed value of one parameter.
    '''
    def __init__ (self,inputname,title=None,label=None):
        self.group_labels = ['opencv_gmm', 'linear', 'staircase', 'ucv_gmm']

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

        self.inputname = inputname
        
        if title == None :
            self.title = "Performance Plot"
        else :
            self.title = title

        # In case label None, it will taken from sequence name
        self.label = label


    def load(self):
        if os.path.exists(self.inputname):
                f = open( self.inputname, 'r' )
                files = f.readlines()
                f.close()

                #remove commented out lines
                lines = [ i for i in files if i[0] != '#' ]

                #ucv_mask_gmm_A_0.001-0.005_T_4-50.txt ; UCV_GMM ; WalkTurnBackPerson1Camera3 ; LearningRate,Threshold ; 0.001 ; 40 
                self.filename  = { i:lines[i].split(';')[0].strip() for i in range(len(lines)) }
                self.algorithm = { i:lines[i].split(';')[1].strip() for i in range(len(lines)) }
                self.sequence  = { i:lines[i].split(';')[2].strip() for i in range(len(lines)) }
                self.columns   = { i:lines[i].split(';')[3].strip() for i in range(len(lines)) }
                self.val_0     = { i:lines[i].split(';')[4].strip() for i in range(len(lines)) }
                self.val_1     = { i:lines[i].split(';')[5].strip() for i in range(len(lines)) }

        else:
            sys.exit()

        if self.label == None :
            self.label = self.sequence[0]


        # Create instances of frameplot
        self.frm = {}
        self.idx = {}
        for i in self.filename.keys() :
            # Takes columns e.g 'LearningRate,Threshold' 
            obj=frameplot( self.columns[i] )
            obj.load(self.filename[i])
            self.idx[i] =  { 'fmeasure' : obj.fm_idx, 'mcc' : obj.mc_idx, 'psnr' : obj.ps_idx, 'mssim' :  obj.ms_idx, 'dscore' : obj.ds_idx } 
            self.frm[i] = obj



        #self.fm_idx  = self.parameters['FMEASURE_MEAN']
        #self.mc_idx  = self.parameters['MCC_MEAN']
        #self.ps_idx  = self.parameters['PSNR']
        #self.ms_idx  = self.parameters['MSSIM']
        #self.ds_idx  = self.parameters['DSCORE']



    def _get_X_Y_Measure(self, idx='fmeasure', val_0=None, val_1=None) :
        '''
        val_0 could be a range of curves: LR 0.001 and 0.003
        val_1 is fixed value of second parameters for example Threshold 40.
        '''
        MSR = {} 
        # Prepare list of indexes for each alg. param
        param = { i : self.idx[i][idx] for i in self.idx.keys() }


        # Obtains pair of X,Y values given by algorithm name and FMEASURE
        # Plot all FMEASURE algorithms with Learning Rate 0.003 and Threshold 40
        for key in self.algorithm.keys() :

            name        = self.algorithm[key]
            line_number = self.val_0    [key]
            param_val   = self.val_1    [key]
            param_idx   = param         [key]

            # Name of curve e.g. LearningRate = 0.001
            if val_0 != None :
                line_number = val_0

            # Value of fixed parameters e.g Threshold = 40 
            if val_1 != None :
                param_val = val_1

            # Returns all values of FMEASURE for a fixed learning rate. LR:0.001 and FMEASURE
            # return for example threshold v/s FMEASURE .
            [X,Y] = self.frm[key]._get_X_Y(line_number, param_idx)

            # Gets FMEASURE for specific value of threshold and associate it to algorithm name.
            MSR[key] = float(Y[X.index(float(param_val))])

        print MSR
        return MSR


    def _plot_one_measure_all_algorithms_together(self,name='fmeasure') :
        '''
        val_0 could be a range of curves: LR 0.001 and 0.003
        val_1 is fixed value of second parameters for example Threshold 40.
        '''

        # Prepare list of measure column index for each alg.
        # { 0 : 5 , 1 : 5 ... }
        param = { i : self.idx[i][name] for i in self.idx.keys() }


        # Counter for line colors
        i=8
        legendP=[]
        legendT=[]

        # Create plot
        plt.clf()
        fig, ax = plt.subplots(nrows=1, ncols=1)

        VAL0_LABEL = {}

        for key in self.algorithm.keys() :

            # Learning Rate v/s Threshold
            [X, Y] = self.frm[key]._get_X_Y(self.val_0[key], param[key] )

            # name of algorithm
            VAL0_LABEL[key] = "%s" % (self.algorithm[key]) 

            # Color counter
            p=int(fmod(i,len(self.colorList)))

            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legendT.append(VAL0_LABEL[key])

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='x')

            i+=1
            #


        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)

        # Prepare bottom label
        self.label = ''
        self.label += "%s=%s" % (self.columns[0].split(',')[0], self.val_0[0]) + "\n"
        self.label += "%s=%s" % (self.columns[0].split(',')[1], self.val_1[0]) + "\n"
        self.label += 'MuHAVI-MAS ' + self.sequence[0]

        ax.text(0.99, 0.05, self.label ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )

        # Prepare axis labels
        xlabel = self.columns[0].split(',')[1]
        ax.set_xlabel(xlabel, fontsize=self.fontsizes[2] )
        #ylabel = name
        #ax.set_ylabel(ylabel, fontsize=self.fontsizes[2] )

        l = plt.axvline(x=self.val_1[0])

        filename = '%s_%s_%s_%s_%s.png' % (name.upper(), \
                                           self.columns[0].split(',')[0][0].upper(), \
                                           self.val_0[0], \
                                           self.columns[0].split(',')[1][0].upper(), \
                                           self.val_1[0])

        ax.set_title(name.upper(), fontsize=self.fontsizes[3] )
        ax.grid()
        plt.savefig(filename)




 
    def plot_performance_summary(self) :

        summary = [ 'fmeasure', 'mcc', 'mssim' ] 
        PLT = {}
        VAL0_LABEL = {}
        VAL1_LABEL = {}

        # Counter for line colors
        i=8
        legendP=[]
        legendT=[]

        # Create plot
        plt.clf()
        fig, ax = plt.subplots(nrows=1, ncols=1)


        for i in range(len(summary)) :

            PLT[i] = self._get_X_Y_Measure(summary[i])

            VAL0_LABEL[i] = "%s" % (summary[i])

            # Color counter
            p=int(fmod(i,len(self.colorList)))

            X = PLT[i].keys()
            Y = [ PLT[i][key] for key in X ]

            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legendT.append(VAL0_LABEL[i])

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='o')

            i+=1

        X = PLT[0].keys()
        LabelsOfAlgorithm = [ self.algorithm[key] for key in X ]
        plt.xticks(X, LabelsOfAlgorithm)

        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)

        # Prepare bottom label
        self.label = ''
        self.label += "%s=%s" % (self.columns[0].split(',')[0], self.val_0[0]) + "\n"
        self.label += "%s=%s" % (self.columns[0].split(',')[1], self.val_1[0]) + "\n"
        self.label += 'MuHAVI-MAS ' + self.sequence[0]

        ax.text(0.99, 0.05, self.label ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )

        #ax.set_xlabel(xlabel, fontsize=self.fontsizes[2] )
        #ax.set_ylabel(ylabel, fontsize=self.fontsizes[2] )
        ax.set_title('Summary Performance Measures', fontsize=self.fontsizes[3] )
        ax.grid()
        plt.savefig('PERFORMANCE.png')

 

    def _plot_performance_measure(self,name='fmeasure',val_0=None, val_1=None) :
        '''
        val_0 could be a range of curves: LR 0.001 and 0.003
        val_1 is fixed value of second parameters for example Threshold 40.
        '''

        parameter_name = name.lower()

        # Verify if val_0 (Learning Rate) is just single or more than one value
        if val_0 != None :
            if isinstance(val_0,list) :
                values_0 = { i:float(val_0[i]) for i in range(len(val_0)) }
            else :
                values_0 = { 0 :float(val_0) }
        else :
            values_0   = { 0 : None }


        PLT = {}
        VAL0_LABEL = {}
        VAL1_LABEL = {}

        # Take as reference first line of files.txt
        if val_1 == None : 
            VAL1_LABEL = { 0 : "%s=%s" % (self.columns[0].split(',')[1], self.val_1[0]) }
        else : 
            VAL1_LABEL = { 0 : "%s=%s" % (self.columns[0].split(',')[1], val_1) }

        for key in values_0.keys() :

            PLT[key] = self._get_X_Y_Measure(parameter_name, values_0[key], val_1)

            if val_0 != None : label_0 = values_0[key]
            else             : label_0 = self.val_0[key]

            VAL0_LABEL[key] = "%s=%s" % (self.columns[key].split(',')[0], label_0) 

        # Counter for line colors
        i=8
        legendP=[]
        legendT=[]

        # Create plot
        plt.clf()
        fig, ax = plt.subplots(nrows=1, ncols=1)


        for val in PLT.keys() :

            # Color counter
            p=int(fmod(i,len(self.colorList)))

            X = PLT[val].keys()
            Y = [ PLT[val][key] for key in X ]

            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            if len(VAL0_LABEL) > 1:
                legendT.append(VAL0_LABEL[val])

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='o')


            i+=1
            #

        X = PLT[0].keys()
        LabelsOfAlgorithm = [ self.algorithm[key] for key in X ]
        plt.xticks(X, LabelsOfAlgorithm)

        if len(VAL0_LABEL) > 1:
            ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)

        # Prepare bottom label
        self.label = ''
        if len(VAL0_LABEL) == 1:
            self.label  = "\n".join(VAL0_LABEL.values()) + "\n"
        self.label += ''.join(VAL1_LABEL.values()) + "\n"
        self.label += 'MuHAVI-MAS ' + self.sequence[0]

        ax.text(0.99, 0.05, self.label ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )

        #ax.set_xlabel(xlabel, fontsize=self.fontsizes[2] )
        #ax.set_ylabel(ylabel, fontsize=self.fontsizes[2] )
        ax.set_title(parameter_name.upper(), fontsize=self.fontsizes[3] )
        ax.grid()
        plt.savefig('SUMMARY_' + parameter_name.upper() +  '.png')

 


    def _plot_mcc(self,val_0=None, val_1=None) :
        '''
        val_0 could be a range of curves: LR 0.001 and 0.003
        val_1 is fixed value of second parameters for example Threshold 40.
        '''

        # Verify if val_0 (Learning Rate) is just single or more than one value
        if val_0 != None :
            if isinstance(val_0,list) :
                values_0 = { i:float(val_0[i]) for i in range(len(val_0)) }
            else :
                values_0 = { 0 :float(val_0) }
        else :
            values_0   = { 0 : None }


        PLT = {}
        VAL0_LABEL = {}
        VAL1_LABEL = {}

        for key in values_0.keys() :

            PLT[key] = self._get_X_Y_Measure('mcc', values_0[key], val_1)

            if val_0 != None : label_0 = values_0[key]
            else             : label_0 = self.val_0[key]
            if val_1 == None : label_1 = self.val_1[key]

            VAL0_LABEL[key] = "%s=%s" % (self.columns[key].split(',')[0], label_0) 
            VAL1_LABEL[key] = "%s=%s" % (self.columns[key].split(',')[1], label_1) 

        # Counter for line colors
        i=8
        legendP=[]
        legendT=[]

        # Create plot
        plt.clf()
        fig, ax = plt.subplots(nrows=1, ncols=1)


        for val in PLT.keys() :

            # Color counter
            p=int(fmod(i,len(self.colorList)))

            X = PLT[val].keys()
            Y = [ PLT[val][key] for key in X ]

            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legendT.append(VAL0_LABEL[val])

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='o')


            i+=1
            #

        X = PLT[0].keys()
        LabelsOfAlgorithm = [ self.algorithm[key] for key in X ]
        plt.xticks(X, LabelsOfAlgorithm)

        ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)
        ax.text(0.99, 0.05, self.label ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )
        #xlabel = 'Names'
        #ylabel = 'FMEASURE'
        print self.label

        #ax.set_xlabel(xlabel, fontsize=self.fontsizes[2] )
        #ax.set_ylabel(ylabel, fontsize=self.fontsizes[2] )
        ax.set_title('MCC', fontsize=self.fontsizes[3] )
        ax.grid()
        plt.savefig('MCC.png')

 


    def _plot_fmeasure(self,val_0=None, val_1=None) :
        '''
        val_0 could be a range of curves: LR 0.001 and 0.003
        val_1 is fixed value of second parameters for example Threshold 40.
        '''

        # Verify if val_0 (Learning Rate) is just single or more than one values
        if val_0 != None :
            if isinstance(val_0,list) :
                values_0 = { i:float(val_0[i]) for i in range(len(val_0)) }
            else :
                values_0 = { 0 :float(val_0) }
        else :
            values_0   = { 0:None }


        MSR = {}
        PLT = {}
        VAL0_LABEL = {}
        VAL1_LABEL = {}

        for idx in values_0.keys() :

            # Obtains pair of X,Y values given by algorithm name and FMEASURE
            for key in self.algorithm.keys() :

                name        = self.algorithm[key]
                line_number = self.val_0    [key]
                param_val   = self.val_1    [key]
                param_idx   = self.frm      [key].fm_idx

                # Name of curve e.g. LearningRate = 0.001
                if values_0[idx] != None :
                    line_number = val_0

                # Value of fixed parameters e.g Threshold = 40 
                if val_1 != None :
                    param_val = val_1

                # Returns all values of FMEASURE for a fixed learning rate. LR:0.001 and FMEASURE
                # return for example threshold v/s FMEASURE .
                [X,Y] = self.frm[key]._get_X_Y(line_number, param_idx)


                # Gets FMEASURE for specific value of threshold and associate it to algorithm name.
                MSR[key] = float(Y[X.index(float(param_val))])

            VAL0_LABEL[idx] = "%s=%s" % (self.columns[idx].split(',')[0], line_number) 
            VAL1_LABEL[idx] = "%s=%s" % (self.columns[idx].split(',')[1], param_val) 

            PLT[idx] = MSR 

        # Counter for line colors
        i=8
        legendP=[]
        legendT=[]

        # Create plot
        plt.clf()
        fig, ax = plt.subplots(nrows=1, ncols=1)


        for val in PLT.keys() :

            # Color counter
            p=int(fmod(i,len(self.colorList)))

            X = PLT[val].keys()
            Y = [ PLT[val][key] for key in X ]
            #LabelsOfAlgorithm = [ str(self.algorithm[key]).lower for key in X ]


            # Save object plot in legendP
            legendP.append(ax.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            #legendT.append(VAL0_LABEL[val])

            ax.plot(X,Y,color=self.colorList[str(p+1)],marker='o')


            i+=1
            #

        X = PLT[0].keys()
        LabelsOfAlgorithm = [ self.algorithm[key] for key in X ]
        plt.xticks(X, LabelsOfAlgorithm)

        #ax.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), prop={'size':8},numpoints=1)
        ax.tick_params(axis='both', which='major', labelsize=8)

        # Prepare bottom label
        self.label  = ''.join(VAL0_LABEL.values()) + "\n"
        self.label += ''.join(VAL1_LABEL.values()) + "\n"
        self.label += 'MuHAVI-MAS ' + self.sequence[0]

        ax.text(0.99, 0.05, self.label ,horizontalalignment='right', verticalalignment='center',  transform=ax.transAxes, fontsize=self.fontsizes[2] )
        #xlabel = 'Names'
        #ylabel = 'FMEASURE'
        print self.label
        print VAL0_LABEL
        print VAL1_LABEL

        #ax.set_xlabel(xlabel, fontsize=self.fontsizes[2] )
        #ax.set_ylabel(ylabel, fontsize=self.fontsizes[2] )
        ax.set_title('F-MEASURE', fontsize=self.fontsizes[3] )
        ax.grid()
        plt.savefig('FMEASURE.png')


    def plot_fmeasure(self,val_0=None, val_1=None) :
        self._plot_performance_measure('fmeasure', val_0, val_1)

    def plot_mcc(self,val_0=None, val_1=None) :
        self._plot_performance_measure('mcc', val_0, val_1)

    def plot_psnr(self,val_0=None, val_1=None) :
        self._plot_performance_measure('psnr', val_0, val_1)

    def plot_mssim(self,val_0=None, val_1=None) :
        self._plot_performance_measure('mssim', val_0, val_1)

    def plot_dscore(self,val_0=None, val_1=None) :
        self._plot_performance_measure('dscore', val_0, val_1)
    


class mergeplots :
    '''
    files: dictionary with list of files  
    '''

    def __init__(self, files, algorithm, columns, values, title=None, label=None):

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
        self.set_title(title,label)

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



def update_file(filename,files,algorithm,sequence,columns,values,second) :
    # Save them in file
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()

    f = open(filename, 'w')
    print >> f,lines[0].strip() 

    for i in range(len( files )):
        line = "%s ; %s ; %s ; %s ; %s ; %s" % (files[i],algorithm[i],sequence[i],columns[i],values[i],second[i])
        print >> f,line
    f.close()





#############
        
if __name__ == '__main__':

    parser = OptionParser ()
    parser.add_option ("-f", "--file",dest = "file",
                       type = "string",
                       default = "files.txt",
                       help = "Select input file, e.g -f files.txt")
    parser.add_option ("-c", "--colums",dest = "colums",
                       type = "string",
                       default = None,
                       help = "Select two colums to be processed, e.g -c '0:Alpha,Threshold'")
    parser.add_option ("-v", "--values",dest = "values",
                       type = "string",
                       default = None,
                       help = "Plot using each value, e.g -v '0.03'")
    parser.add_option ("-1", "--first",dest = "first",
                       type = "string",
                       default = None,
                       help = "Plot performance measure using list if this value,just valid for performance e.g -1 '0.001 0.002 0.003'")
    parser.add_option ("-2", "--second",dest = "second",
                       type = "string",
                       default = None,
                       help = "Plot performance measure using this value, e.g -2 '40'")
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
    filename = options.file
    if os.path.exists(filename):
            f = open( filename, 'r' )
            files = f.readlines()
            f.close()

            #remove first line
            lines = [ i for i in files if i[0] != '#' ]

            files     = {i:lines[i].split(';')[0].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            algorithm = {i:lines[i].split(';')[1].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            sequence  = {i:lines[i].split(';')[2].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            columns   = {i:lines[i].split(';')[3].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            values    = {i:lines[i].split(';')[4].strip() for i in range(len(lines)) if lines[i][0] != '#' }
            second    = {i:lines[i].split(';')[5].strip() for i in range(len(lines)) if lines[i][0] != '#' }

    else:
        sys.exit()

    if options.values != None :
        # Update values from console
        for i in values.keys() : 
            values[i] = options.values

        # Save them in file
        update_file(filename, files,algorithm,sequence,columns,values,second)

    if options.colums != None :
        # Update columns from console
        for i in columns.keys() :
            columns[i] = options.columns

        # Save them in file
        update_file(filename, files,algorithm,sequence,columns,values,second)

    title = options.title
    if options.label != None :
        label = options.label
        f = open('mergeplot_labels.txt','w')
        print >> f,label
        f.close()
    else :    
        if os.path.exists('mergeplot_labels.txt') :
            f = open('mergeplot_labels.txt','r')
            lines = f.readlines()
            f.close()
            label = ''.join(lines)
        else :
            label = "MuHAVI-MAS %s" % (sequence[0])
            f = open('mergeplot_labels.txt','w')
            print >> f,label
            f.close()

    n = mergeplots(files, algorithm, columns, values, title, label) 
    n.load()
    n.plot1()



    # Preparing values
    val = None
    if options.first != None :
        val = [ float(i) for i in options.first.split() ]

    if options.second != None :
        # Update values from console
        for i in second.keys() :
            second[i] = options.second

        # Save them in file
        update_file(filename, files,algorithm,sequence,columns,values,second)

    

    m = msplot(filename,title,label)
    m.load()
    m.plot_fmeasure(val)
    m.plot_mcc(val)
    m.plot_psnr(val   )
    m.plot_mssim(val  )
    m.plot_dscore( val)
    m.plot_performance_summary()
    m._plot_one_measure_all_algorithms_together('fmeasure') 
    m._plot_one_measure_all_algorithms_together('mcc') 
    m._plot_one_measure_all_algorithms_together('psnr') 
    m._plot_one_measure_all_algorithms_together('mssim') 
    m._plot_one_measure_all_algorithms_together('dscore') 

