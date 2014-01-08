#!/usr/bin/python

import sys, getopt
import numpy as np
from os import listdir
from os import system
from os.path import isfile, join
from optparse import OptionParser
import re



class ListFiles:
    '''Get a list of files of a specific directory'''
    def __init__(self,inputdir):
        self.path   = inputdir

    def getList(self):
        only_files = [f for f in listdir(self.path) if isfile(join(self.path,f)) and (re.search("output.*txt",f) or re.search("matlab.*txt",f))]
        return only_files


class ParseHeader:
    '''Parse parameters included in header of file'''
    def __init__(self, inputline):
        self.line = inputline
        self.Parameters = {}

    def getDictionary(self):
        if self.line != '':
            self.Parameters = {f.split('=')[0]:f.split('=')[1] for f in self.line.split() if f != "#"}
        return self.Parameters

    def getNames(self):
        if self.line != '':
            return [f.split('=')[0] for f in self.line.split() if f != "#"]

    def getValues(self):
        if self.line != '':
            return [f.split('=')[1] for f in self.line.split() if f != "#"]


class ParseLine:
    '''Parse any line of input file'''
    def __init__(self, inputline):
        self.line = inputline

    def getValues(self):
        if self.line != '':
            return [i for i in self.line.split() if i != '#']


class ParseFile:
    '''Read output files after executing bgs algortihm'''

    def __init__(self, inputfile):
        self.filename = inputfile

    def getQualityMetrics(self):
        f = open(self.filename)
        lines = f.readlines()
        f.close()
        size=len(lines)
        header     = ParseHeader(lines[0]).getDictionary()
        parameters = [i for i in lines[size-2].split() if i != '#']
        values     = [i for i in lines[size-1].split() if i != '#']
        names      = "# " + " ".join(ParseHeader(lines[0]).getNames()) + \
                     " " + " ".join(ParseLine(lines[size-2]).getValues()) 
        values = " ".join([ParseHeader(lines[0]).getDictionary()[i] for i in ParseHeader(lines[0]).getNames()]) +\
                 " " +\
                 " ".join(ParseLine(lines[size-1]).getValues())
        return names,values

class PerformanceMeasureFile:
    '''Creates a final final containing all performance measures'''

    def __init__(self,inputpath):
        self.path     = inputpath.rstrip('/')
        length        =  len(self.path.split('/'))
        self.filename = self.path.split('/')[length-1] + '.txt'
    def write(self):
        files = ListFiles(self.path).getList()
        header = ParseFile(join(self.path,files[0])).getQualityMetrics()[0]
        f = open(self.filename ,'w')
        print >> f,header
        f.writelines( "%s\n" % ParseFile(join(self.path,item)).getQualityMetrics()[1] for item in files )
        f.close()


if __name__ == '__main__':

    parser = OptionParser ()
    parser.add_option ("-p", "--path",dest = "path",
                        type = "string",
                        default = None,
                        help = "Select input directory, e.g -p results")
    (options, args) = parser.parse_args ()

    if options.path == None:
        parser.print_help()
        exit()

    PerformanceMeasureFile(options.path).write()

