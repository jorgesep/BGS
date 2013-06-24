#!/opt/local/bin/python

import sys, getopt
import numpy as np
from os import listdir
from os.path import isfile, join
from optparse import OptionParser

config= '''
set border 3
set yrange[0.5:0.9]
set mytics 10
set key off
set key below
set grid
set term png
set style data linespoints
plot datafile u 3:2 t "12" w linespoints, datafile u (($3)*1.05):(($2)*0.99):1 title ''  w labels, \\n
     for [i=4:42:2] datafile u (column(i+1)):(column(i)) w l lw 1.5 t 'Range'.(i+10)
'''



def inputOptions ():
    parser = OptionParser ()
    parser.add_option ("-p", "--path",
                       dest = "path",
                       type = "string",
                       default = None,
                       help = "Select input directory, e.g -p results/2013-06-06_23-30-30")
    parser.add_option ("-m", "--median",
                       dest = "median",
                       action ="store_true",
                       help = "Select median instead of mean results as default.")
    parser.add_option ("-t", "--transpose",
                       dest = "transpose",
                       action ="store_true",
                       help = "Create transpose file table")

    (options, args) = parser.parse_args ()
    return [options.path,options.median,options.transpose]

def create_roc_gnuplot_configuration_file(_filename,_title=None):

    #line = "nset title %s\nset xlabel %s\nset ylabel %s\nset output %s" % (_filename,_title,_xlabel,_ylabel,outfile)
    _outfile=_filename.strip('.txt') + ".cfg"
    _pngfile=_filename.strip('.txt') + ".png"

    print _outfile

    strline= '''datafile="%s" 
set border 3
set yrange[0.5:0.9]
set mytics 10
set key off
set key below
set grid
set title "%s"
set xlabel "False Positive Rate"
set ylabel "True Positive Rate"
set term png
set output "%s"
set style data linespoints
plot datafile u 3:2 t "12" w linespoints, datafile u (($3)*1.05):(($2)*0.99):1 title ''  w labels, \\
     for [i=4:42:2] datafile u (column(i+1)):(column(i)) w l lw 1.5 t 'Range'.(i+10)
''' % (_filename, _title, _pngfile)

    _file   = open(_outfile, "w")
    print >> _file, strline
    _file.close()

def create_mcc_gnuplot_configuration_file(_filename,_title=None,_x=None,_y=None,_ylabels=None):

    _outfile=_filename.strip('.txt') + ".cfg"
    _pngfile=_filename.strip('.txt') + ".png"

    #labelname(n) = sprintf("%s", _ylabels[n])

    if _ylabels == None:
        cnt=3
    else:
        cnt=len(_ylabels)

    #print _outfile

    strline= '''datafile="%s" 
set border 3
#set yrange[0.5:0.9]
set mytics 10
set key off
set key below
set grid
set title "%s"
set xlabel "%s"
set ylabel "%s"
set term png
set output "%s"
set style data linespoints
plot for [i=2:%d] datafile u i:1 w lp lw 1.5 t 'column '.(i)
''' % (_filename, _title, _x,_y,_pngfile,cnt)

    _file   = open(_outfile, "w")
    print >> _file, strline
    _file.close()




def get_lists( _path, _transpose=False):

    onlyfiles = [ f for f in listdir(_path) if isfile(join(_path,f)) and f.find('output_') == 0]

    _tmp1= list(set([f.strip('output_.txt').split('_')[0] for f in onlyfiles]))
    _tmp2= list(set([f.strip('output_.txt').split('_')[1] for f in onlyfiles]))

    if _transpose:
        _t1   = _tmp1
        _tmp1 = _tmp2
        _tmp2 = _t1

    _tmp1.sort()
    _tmp2.sort()

    #verify if numbers are float or integer 
    if _tmp1[0].find('.') < 0:
        _list_1 = [int(f) for f in _tmp1]
    else:
        _list_1 = [f for f in _tmp1]
    
    if _tmp2[0].find('.') < 0:
        _list_2 = [int(f) for f in _tmp2]
    else:
        _list_2 = [f for f in _tmp2]


    _list_1.sort()
    _list_2.sort()

    return [_list_1,_list_2]
 
def roctable(_path):
    '''Create roc.txt file'''

    [_list_1,_list_2] = get_lists(_path)

    meanfile   = open('roc_mean.txt', "w")
    medianfile = open('roc_median.txt', "w")

    head  = "# " + " ".join([str(i) + " " + str(i) for i in _list_2])
    print >> meanfile  , head
    print >> medianfile, head

    _title = ""

    for i in _list_1:
        line1 = "%s " % (i)
        line2 = "%s " % (i)
        for j in _list_2:

            file="%s/output_%s_%s.txt" % (_path,str(i),str(j))

            f = open(file)
            _list = f.readlines()
            f.close()

            if _title == "": 
                _title = " ".join([k for k in _list[0].strip('#\n').split() if k.find(str(i)) < 0 \
                        and k.find(str(j)) < 0 \
                        and k.find("GaussiansNo") < 0 \
                        and k.find("Sigma") < 0 ])

            line1 = "%s %s %s " % (line1, 
                    _list[len(_list)-1].strip('#').split()[0], 
                    _list[len(_list)-1].strip('#').split()[1])
            line2 = "%s %s %s " % (line2, 
                    _list[len(_list)-1].strip('#').split()[4], 
                    _list[len(_list)-1].strip('#').split()[5])
   
        print >> meanfile  ,line1
        print >> medianfile,line2
        #print line

    meanfile.close()
    medianfile.close()
##
    #print _title
    create_roc_gnuplot_configuration_file('roc_median.txt', "MuHAVI Median ROC Curve\\n" + _title)
    create_roc_gnuplot_configuration_file('roc_mean.txt', "MuHAVI Mean ROC Curve\\n" + _title)

#plot 'roc_mean.txt' u 3:2 w linespoints, 'roc_mean.txt' u 3:2:1 w labels, 'roc_mean.txt' u 5:4 w linespoints, 'roc_mean.txt' u 7:6 w linespoints, 'roc_mean.txt' u 9:8 w linespoints, 'roc_mean.txt' u 11:10 w linespoints, 'roc_mean.txt' u 13:12 w linespoints, 'roc_mean.txt' u 15:14 w linespoints, 'roc_mean.txt' u 15:14:1 w labels


def mcctable(_path,_inv=False):
  
    [_list_1,_list_2] = get_lists(_path, _inv)

    _var_x=str(_list_1[0])
    _var_y=str(_list_2[0])

    _pa_1 = 'param1'
    _pa_2 = 'param2'
    if _inv:
        _pa_1 = 'param2'
        _pa_2 = 'param1'

    filename = _pa_1 + '_' + _pa_2
    meanfile   = open(filename + '_mean.txt', "w")
    medianfile = open(filename + '_median.txt', "w")

    print >> meanfile,   "#" + _pa_1 + " " +  " ".join([str(i) for i in _list_1])
    print >> meanfile,   "#" + _pa_2 + " " +  " ".join([str(i) for i in _list_2]) 

    print >> medianfile, "#" + _pa_1 + " " +  " ".join([str(i) for i in _list_1])
    print >> medianfile, "#" + _pa_2 + " " +  " ".join([str(i) for i in _list_2]) 

    _title = ""
    _x = ""
    _y = ""

    for i in _list_1:
        line1 = "%s " % (i)
        line2 = "%s " % (i)
        for j in _list_2:
            if _inv:
                file="%s/output_%s_%s.txt" % (path,str(j),str(i))
            else:
                file="%s/output_%s_%s.txt" % (path,str(i),str(j))

            f = open(file)
            _list = f.readlines()
            f.close()

            if _x == "" and _y == "":
                _x      = [k for k in _list[0].split() if k.find(_var_x)>0][0].split('=')[0]
                _y      = [k for k in _list[0].split() if k.find(_var_y)>0][0].split('=')[0]

            if _title == "": 
                _title = " ".join([k for k in _list[0].strip('#\n').split() if k.find(str(i)) < 0 \
                        and k.find(str(j)) < 0 \
                        and k.find("GaussiansNo") < 0 \
                        and k.find("Sigma") < 0 ])

            #get Mean values
            if _list[len(_list)-1].strip("# \n").split()[3] == '-nan':
                _val = 0
            else:
                _val = float(_list[len(_list)-1].strip("# \n").split()[3])
   
            line1 = "%s %s " % (line1,str(_val))
    
            #get Median values
            if _list[len(_list)-1].strip("# \n").split()[7] == '-nan':
                _val = 0
            else:
                _val = float(_list[len(_list)-1].strip("# \n").split()[7])
   
            line2 = "%s %s " % (line2,str(_val))

        print >> meanfile,line1
        print >> medianfile,line2
        print line1

    meanfile.close()
    medianfile.close()

    _title = "MCC Curve\\n" + _title
    #print _title
    _ylabels=[_y+'='+str(k) for k in _list_2]
    #print _ylabels
    create_mcc_gnuplot_configuration_file( filename+"_mean.txt","Mean " + _title,_x,_y,_ylabels)
    create_mcc_gnuplot_configuration_file( filename+"_median.txt","Median " + _title,_x,_y,_ylabels)

 


if __name__ == '__main__':
    [path,_median,_inv] = inputOptions()

    if path == None:
        path='.'
        
    mcctable(path)
    mcctable(path,True)

    roctable(path)

#    if path == None:
#        path='.'
#
#    if _median:
#        _type='median'
#        _col=7
#    else:
#        _type='mean'
#        _col=3
#
#    onlyfiles = [ f for f in listdir(path) if isfile(join(path,f)) and f.find('output_') == 0]
#
#    _tmp1= list(set([f.strip('output_.txt').split('_')[0] for f in onlyfiles]))
#    _tmp2= list(set([f.strip('output_.txt').split('_')[1] for f in onlyfiles]))
#
#    filename = "param1_param2_" + _type + ".txt"
#    if _inv:
#        _t1   = _tmp1
#        _tmp1 = _tmp2
#        _tmp2 = _t1
#        filename = "param2_param1_" + _type + ".txt"
#
#    _tmp1.sort()
#    _tmp2.sort()
#
#    #verify if numbers are float or integer 
#    if _tmp1[0].find('.') < 0:
#        _list_1 = [int(f) for f in _tmp1]
#    else:
#        _list_1 = [f for f in _tmp1]
#    
#    if _tmp2[0].find('.') < 0:
#        _list_2 = [int(f) for f in _tmp2]
#    else:
#        _list_2 = [f for f in _tmp2]
#
#
#    _list_1.sort()
#    _list_2.sort()
#    
#    outfile = open(filename, "w")
#    if _inv:
#        print >> outfile, "#param1 " + " ".join([str(i) for i in _list_1])
#        print >> outfile, "#param2 " + " ".join([str(i) for i in _list_2]) 
#    else:
#        print >> outfile, "#param2 " + " ".join([str(i) for i in _list_2])
#        print >> outfile, "#param1 " + " ".join([str(i) for i in _list_1]) 
#
#    for i in _list_1:
#        line = "%s " % (i)
#        for j in _list_2:
#            if _inv:
#                file="%s/output_%s_%s.txt" % (path,str(j),str(i))
#            else:
#                file="%s/output_%s_%s.txt" % (path,str(i),str(j))
#
#            f = open(file)
#            _list = f.readlines()
#            f.close()
#
#            if _list[len(_list)-1].split()[_col] == '-nan':
#                _val = 0
#            else:
#                _val = float(_list[len(_list)-1].split()[_col])
#   
#            line = "%s %s " % (line,str(_val))
#    
#        print >> outfile,line
#        print line
#    outfile.close()



#lines = [line.strip() for line in open(fileName)]




#print sys.argv[0]
#
#if len(sys.argv) < 2:
#    sys.exit("Insert path to output files.")
#
#path=sys.argv[1]
#
#_inverse = True
#onlyfiles = [ f for f in listdir(path) if isfile(join(path,f)) and f.find('output_') == 0]
#
#_tmp1= list(set([f.strip('output_.txt').split('_')[0] for f in onlyfiles]))
#_tmp2= list(set([f.strip('output_.txt').split('_')[1] for f in onlyfiles]))
#
#_tmp1.sort()
#_tmp2.sort()
#filename = "param1_param2.txt"
#if _inverse:
#    _t1   = _tmp1
#    _tmp1 = _tmp2
#    _tmp2 = _t1
#    filename = "param2_param1.txt"
#
#
#if _tmp1[0].find('.') < 0:
#    _list_1 = [int(f) for f in _tmp1]
#else:
#    _list_1 = [f for f in _tmp1]
#
#if _tmp2[0].find('.') < 0:
#    _list_2 = [int(f) for f in _tmp2]
#else:
#    _list_2 = [f for f in _tmp2]
#
#
#
#_list_1.sort()
#_list_2.sort()
#
#outfile = open(filename, "w")
#if _inverse:
#    print >> outfile, "#param1 " + " ".join([str(i) for i in _list_1])
#    print >> outfile, "#param2 " + " ".join([str(i) for i in _list_2]) 
#else:
#    print >> outfile, "#param2 " + " ".join([str(i) for i in _list_2])
#    print >> outfile, "#param1 " + " ".join([str(i) for i in _list_1]) 
#
#for i in _list_1:
#    line = "%s " % (i)
#    for j in _list_2:
#        if _inverse:
#            file="%s/output_%s_%s.txt" % (path,str(j),str(i))
#        else:
#            file="%s/output_%s_%s.txt" % (path,str(i),str(j))
#        f = open(file)
#        _list = f.readlines()
#        f.close()
#        if _list[len(_list)-1].split()[3] == '-nan':
#            mean = 0
#        else:
#            mean = float(_list[len(_list)-1].split()[3])
#        if _list[len(_list)-1].split()[7] == '-nan':
#            median = 0
#        else:
#            median = float(_list[len(_list)-1].split()[7])
#
#        line = "%s %s " % (line,str(mean))
#
#    print >> outfile,line
#    #print line
#outfile.close()
#
#
#
