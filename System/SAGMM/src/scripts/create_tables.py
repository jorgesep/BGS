#!/opt/local/bin/python

import sys, getopt
import numpy as np
from os import listdir
from os import system
from os.path import isfile, join
from optparse import OptionParser



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
set mxtics 10
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



def create_general_gnuplot_configuration_for_roc_curve(_title):
    meanfile="roc_mean.txt" 
    medianfile="roc_median.txt" 
    cfgfile="roc_curve.cfg" 
    pngfile="roc_curve.png" 

    strline= '''meanfile="%s"
medianfile="%s"
set size ratio 0.71 # this is the ratio of a DIN A4 page (21/29.7)
set border 3
set yrange[0.5:0.9]
set mytics 10
set mxtics 10
set key off
set key below
set grid
#set terminal png enhanced size 1024,768
set terminal png enhanced font arial 14 size 768,1024
set output "%s"
set style data linespoints
set xlabel "False Positive Rate"
set ylabel "True Positive Rate"
set multiplot layout 2, 1 title  "%s"
set title "Mean curve"
plot meanfile u 3:2 t "Range=12" w lp, meanfile u (($3)*1.05):(($2)*0.99):1 t ''  w labels, \\
     for [i=4:12:2] meanfile u (column(i+1)):(column(i)) w lp lw 1.5 t 'Range'.(i+10)
set title "Median curve"
plot medianfile u 3:2 t "12" w lp, medianfile u (($3)*1.05):(($2)*0.99):1 title ''  w labels, \\
     for [i=4:12:2] medianfile u (column(i+1)):(column(i)) w lp lw 1.5 t 'Range'.(i+10)
''' % (meanfile,medianfile,pngfile,_title)

    _file   = open(cfgfile, "w")
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
set mytics 10
set mxtics 5
set key off
set key below
set grid
set title "%s"
set xlabel "%s"
set ylabel "%s"
set term png
set output "%s"
set style data linespoints
plot for [i=2:%d] datafile u 1:i w lp lw 1.5 t 'column '.(i)
''' % (_filename, _title, _x,_y,_pngfile,cnt)

    _file   = open(_outfile, "w")
    print >> _file, strline
    _file.close()




def create_general_gnuplot_configuration_for_mcc_curve(_title,_desc,_inv=True):

    if _inv:
        _mean   ="param2_param1_mean.txt"
        _median ="param2_param1_median.txt"
        _cfg    ="mcc_param2_param1.cfg" 
        _png    ="mcc_param2_param1.png" 
    else:
        _mean   ="param1_param2_mean.txt"
        _median ="param1_param2_median.txt"
        _cfg    ="mcc_param1_param2.cfg" 
        _png    ="mcc_param1_param2.png" 

    # create plot lines
    #print len(_desc.split())
    _nl = 4
    if _nl > len(_desc.split()):
        _nl = len(_desc.split()[1:])

    _plmean   = ", ".join(["meanfile u 1:%d w lp lw 1.5 t '%s'"  %(i+2,str(_desc.split()[1:][i])) for i in range(_nl)])
    _plmedian = ", ".join(["medianfile u 1:%d w lp lw 1.5 t '%s'"%(i+2,str(_desc.split()[1:][i])) for i in range(_nl)])

    strline= '''meanfile="%s"
medianfile="%s"
set size ratio 0.71 # this is the ratio of a DIN A4 page (21/29.7)
set border 3
set mytics 10
set mxtics 5
set key off
set key below
set grid
set terminal png enhanced font arial 14 size 768,1024
set output "%s"
set style data linespoints
set xlabel "Range"
set ylabel "MCC"
set multiplot layout 2, 1 title  "%s"
set title "Mean curve"
plot %s
set title "Median curve"
plot %s
''' % (_mean,_median,_png,_title,_plmean,_plmedian)

    _file   = open(_cfg, "w")
    print >> _file, strline
    _file.close()


def get_lists( _path, _transpose=False):

    onlyfiles = [ f for f in listdir(_path) if isfile(join(_path,f)) and f.find('output_') == 0 or f.find('matlab_output_') == 0]

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
                        and k.find(str(float(i))) < 0 \
                        and k.find(str(float(j))) < 0 \
                        and k.find(str(i)) < 0 \
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
    create_roc_gnuplot_configuration_file('roc_median.txt', "MuHAVI Median ROC Curve\\n" + _title)
    create_roc_gnuplot_configuration_file('roc_mean.txt', "MuHAVI Mean ROC Curve\\n" + _title)
    create_general_gnuplot_configuration_for_roc_curve("MuHAVI ROC Curve \\n" + _title)
    system('gnuplot roc_median.cfg')
    system('gnuplot roc_mean.cfg')
    system('gnuplot roc_curve.cfg')



def mcctable(_path,_inv=False):
  
    [_list_1,_list_2] = get_lists(_path, _inv)

    _var_x=str(_list_1[0])
    _var_y=str(_list_2[0])
    # put both numbers in scientific notation
    _vx_sn = str(float(_var_x))
    _vy_sn = str(float(_var_y))

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
                print "Opening %s" % (file)
                _x      = [k for k in _list[0].split() if k.find(_var_x)>0 or k.find(_vx_sn)>0][0].split('=')[0]
                _y      = [k for k in _list[0].split() if k.find(_var_y)>0 or k.find(_vy_sn)>0][0].split('=')[0]

            if _title == "": 
                #print _list[0]
                _title = " ".join([k for k in _list[0].strip('#\n').split() if k.find(str(i)) < 0 \
                        and k.find(str(float(i))) < 0 \
                        and k.find(str(float(j))) < 0 \
                        and k.find(str(i)) < 0 \
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
        #print line1

    meanfile.close()
    medianfile.close()

    _title = "Matthews Correlation Coefficient\\n" + _title
    #print _title
    _ylabels=[_y+'='+str(k) for k in _list_2]
    #print _ylabels
    create_mcc_gnuplot_configuration_file( filename+"_mean.txt","Mean " + _title,_x,_y,_ylabels)
    create_mcc_gnuplot_configuration_file( filename+"_median.txt","Median " + _title,_x,_y,_ylabels)

    f = open(filename + "_mean.txt")
    _list = f.readlines()
    f.close()

    #print "LIST 1 %s" % (_list[1])
    create_general_gnuplot_configuration_for_mcc_curve(_title,_list[1],_inv)


    if _inv:
        system('gnuplot param2_param1_mean.cfg') 
        system('gnuplot param2_param1_median.cfg')
        system('gnuplot mcc_param2_param1.cfg')
    else:
        system('gnuplot param1_param2_mean.cfg') 
        system('gnuplot param1_param2_median.cfg') 
        system('gnuplot mcc_param1_param2.cfg')


if __name__ == '__main__':
    [path,_median,_inv] = inputOptions()

    if path == None:
        path='.'
        
    mcctable(path)
    mcctable(path,True)

    roctable(path)


