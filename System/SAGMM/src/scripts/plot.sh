#! /opt/local/bin/gnuplot

clear
reset
set key off
set border 3
set auto
#set xrange[170:255]
set xtics 5
set title "Background"
set xlabel "Red"
set ylabel "Count"
ft="png"
set output "enes.".ft
set style histogram clustered gap 1
set style fill solid border -1
#binwidth=1000
#binwidth=10
#binwidth=5
binwidth=2
#binwidth=1
set boxwidth binwidth
bin(x,width)=width*floor(x/width) + binwidth/2.0
#plot 'output.txt' using (bin($22,binwidth)):(1.0) smooth freq with boxes
#plot 'output.txt' using (bin($8,binwidth)):(1.0) smooth freq with boxes
#plot  'p_680_200.txt' u (bin($1,binwidth)):(1.0) smooth freq with boxes
plot  'pnof_680_200.txt' u (bin($3,binwidth)):(1.0) smooth freq with boxes

