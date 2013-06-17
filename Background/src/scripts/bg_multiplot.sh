#! /opt/local/bin/gnuplot

clear
reset
ft="png"
binwidth=2
datafile="pwf_680_200.txt"
bin(x,width)=width*floor(x/width) + binwidth/2.0
gauss(x)=amplitude/(sigma*sqrt(2.*pi))*exp(-(x-position)**2/(2.*sigma**2))
set output "pt_680_200.".ft
set xrange[130:170]
set multiplot layout 3, 1 title "Point 680,200"
#set title "Pixel BLUE"
set key off
set border 3
set auto
set xtics 5
set xlabel "blue"
set ylabel "Count"
set style histogram clustered gap 1
set style fill solid border -1
set boxwidth binwidth
amplitude=150
sigma=3.15
position=103
fit gauss(x) datafile using (bin($1,binwidth)):(1.0) via position,amplitude,sigma
#fit gauss(x) datafile using ($1):(1.0) via position,amplitude,sigma
plot datafile u (bin($1,binwidth)):(1.0) smooth freq with boxes, \
     datafile , gauss(x) title "Gaussian-Fit Curve"
#set title "Pixel GREEN"
set key off
set border 3
set auto
set xtics 5
set xlabel "green"
set ylabel "Count"
set style histogram clustered gap 1
set style fill solid border -1
set boxwidth binwidth
plot  datafile u (bin($2,binwidth)):(1.0) smooth freq with boxes
#set title "Pixel RED"
set key off
set border 3
set auto
set xtics 5
set xlabel "red"
set ylabel "Count"
set style histogram clustered gap 1
set style fill solid border -1
set boxwidth binwidth
plot  datafile u (bin($3,binwidth)):(1.0) smooth freq with boxes
unset multiplot
