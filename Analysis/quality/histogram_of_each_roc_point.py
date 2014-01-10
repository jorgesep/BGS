#!/usr/bin/python

from readfiles import ListFiles, ParseHeader, ParseLine, ParseFile 
from optparse import OptionParser
from os.path import isfile, join
import sys
import re

from pylab import *
from numpy import *
from scipy.optimize import leastsq
import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse

from mpl_toolkits.mplot3d import Axes3D


if __name__ == '__main__':

    parser = OptionParser ()
    parser.add_option ("-p", "--path",dest = "path",
                       type = "string",
                       default = None,
                       help = "Select input path, e.g -f pt_340_160.txt")
    parser.add_option ("-a", "--alpha",dest = "_alpha",
                       type = "float",
                       default = 0.001,
                       help = "Select Learning Rate: -a 0.001")

    (options, args) = parser.parse_args ()

    if options.path == None :
        parser.print_help()
        sys.exit()

    path = options.path.rstrip('/')    

    rgb     = {0:'Blue',1:'Green',2:'Red'} 
    colors_ = {0:'m', 1:'c', 2:'y', 3:'0.75', 4:'c', 5:'m', 6:'k' , 7:'w' }


    files = ListFiles(path).getList()

    #pattern = "A_" + str(options._alpha) + ".*T_2" + "|" +  "L_" + str(options._alpha) + ".*T_?[0-9]"
    #pattern = "A_" + str(options._alpha)  + "|" +  "L_" + str(options._alpha) 
    pattern = "A_" + str(options._alpha)  + "|" +  "L_" + str(options._alpha) + ".*T_[1-2]?1[0-6]" 

    alpha_list = [ i for i in files if re.search(pattern, i) ]


    for k in range(len(alpha_list)) :
        # get parameters name
        a=[ i for i in ParseFile( join(path,files[k]) ).getQualityMetrics()[0].split() if i != '#' ]

        # get its values
        b=ParseFile( join(path,files[k]) ).getQualityMetrics()[1].split()

        a_b = { a[i] : b[i] for i in range(len(a)) }

        Threshold = 0
        if a_b.has_key('Threshold') :
            Threshold = int(a_b['Threshold'])
        else :
            Threshold = int(a_b['Range'])  

        #print a_b
        #sys.exit()

        if Threshold <= 16 : 

            #reading data
            print join(path,alpha_list[k])
            data   = loadtxt(join(path,alpha_list[k]), usecols = (7,8), dtype='float')
            SEN    = data[:,0]
            ESP    = 1 - data[:,1]
            num_bins   = 70
            a,  b,  c    = hist(SEN,num_bins,range=None,normed=1,histtype='step' )
            e,  f,  g    = hist(ESP,num_bins,range=None,normed=1,histtype='step' )

            xdata   = b[:-1] # this command remove last element of array
            ydata   = a

            xdata1  = f[:-1] # this command remove last element of array
            ydata1  = e



            # two plots.
            plt.clf()
            fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2)

            ax1.grid()
            ax1.set_ylabel('Frequencia' )
            ax1.set_xlabel('Sensitivity')
            #ax1.plot(xdata, ydata , linestyle='-', linewidth=1 , color='k')
            #n, bins, patches = plt.hist(SEN, num_bins, normed=1, facecolor='green', alpha=0.5)
            n, bins, patches = ax1.hist(SEN, num_bins, normed=1, facecolor='green', alpha=0.9)
            ax1.tick_params(axis='both', which='major', labelsize=10)



            ax2.grid()
            ax2.set_ylabel('Frequencia' )
            ax2.set_xlabel('Specificity')
            #ax2.plot(xdata1, ydata1 , linestyle='-', linewidth=1 , color='k')
            n, bins, patches = ax2.hist(ESP, num_bins, normed=1, facecolor='green', alpha=0.5)
            ax2.tick_params(axis='both', which='major', labelsize=10)

            fig.set_tight_layout(True)
            filename = str(options._alpha) + '_' + str(Threshold) + '.png'
            plt.savefig(filename)





