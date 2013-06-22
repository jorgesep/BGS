#!/opt/local/bin/python

import sys, getopt
import numpy as np
from os import listdir
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
    parser.add_option ("-i", "--inverse",
                       dest = "inverse",
                       action ="store_true",
                       help = "Create an inverted file table")

    (options, args) = parser.parse_args ()
    return [options.path,options.median,options.inverse]

if __name__ == '__main__':
    [path,_median,_inv] = inputOptions()

    if path == None:
        path='.'

    if _median:
        _type='median'
        _col=7
    else:
        _type='mean'
        _col=3

    onlyfiles = [ f for f in listdir(path) if isfile(join(path,f)) and f.find('output_') == 0]

    _tmp1= list(set([f.strip('output_.txt').split('_')[0] for f in onlyfiles]))
    _tmp2= list(set([f.strip('output_.txt').split('_')[1] for f in onlyfiles]))

    filename = "param1_param2_" + _type + ".txt"
    if _inv:
        _t1   = _tmp1
        _tmp1 = _tmp2
        _tmp2 = _t1
        filename = "param2_param1_" + _type + ".txt"

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
    
    outfile = open(filename, "w")
    if _inv:
        print >> outfile, "#param1 " + " ".join([str(i) for i in _list_1])
        print >> outfile, "#param2 " + " ".join([str(i) for i in _list_2]) 
    else:
        print >> outfile, "#param2 " + " ".join([str(i) for i in _list_2])
        print >> outfile, "#param1 " + " ".join([str(i) for i in _list_1]) 

    for i in _list_1:
        line = "%s " % (i)
        for j in _list_2:
            if _inv:
                file="%s/output_%s_%s.txt" % (path,str(j),str(i))
            else:
                file="%s/output_%s_%s.txt" % (path,str(i),str(j))

            f = open(file)
            _list = f.readlines()
            f.close()

            if _list[len(_list)-1].split()[_col] == '-nan':
                _val = 0
            else:
                _val = float(_list[len(_list)-1].split()[_col])
   
            line = "%s %s " % (line,str(_val))
    
        print >> outfile,line
        #print line
    outfile.close()




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
