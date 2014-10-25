#!/usr/bin/gnuplot
clear
reset
set grid
set xrange[0:0.02]
plot "roc_10.txt" u 8:7  w lines, "roc_20.txt" u 8:7  w lines,"roc_30.txt" u 8:7  w lines, "roc_40.txt" u 8:7  w lines, "roc_50.txt" u 8:7  w lines, "roc_60.txt" u 8:7  w lines, "roc_70.txt" u 8:7 w lines , "roc_80.txt" u 8:7  w lines, "roc_90.txt" u 8:7 w lines, "roc_100.txt" u 8:7 w lines
