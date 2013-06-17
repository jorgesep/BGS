#!/bin/bash

DATE=`date "+%Y-%m-%d_%H:%M:%S"`
DST="results/`date "+%Y-%m-%d_%H-%M-%S"`"
echo $output_str

if [ ! -d "$DST" ]; then
    mkdir -p $DST
fi

INPUT=$1
GT=$2

if [ "$INPUT" == "" ]
then
    echo "Not input video."
    exit
fi

if [ "$GT" == "" ]
then
    args="-i $INPUT -c init.txt"
else
    args="-i $INPUT -g $GT -c init.txt"
fi

cmd="./bin/bgs"

alpha_list=`cat config/Alpha.txt | awk '{print $3}'`

for i in $alpha_list
do
    echo "Processing alpha: $i"
    cat config/init.txt | grep -v Alfa > init.txt
    echo "Alfa : $i" >> init.txt
    $cmd $args
    mv output.txt $DST/output_$i.txt
done

#cat Alpha.txt | while read in; do date; file=`echo $in | awk '{print $3}'`; echo output_$file.txt; cat init.txt | grep -v Alfa > init.tmp; echo $in >> init.tmp; mv init.tmp init.txt; ./bin/bgs -i /Users/jsepulve/Tesis/Videos/WalkTurnBack-Camera_3-Person1.avi -g /Users/jsepulve/Tesis/Code/Matlab/MuHavi/WalkTurnBack/Person1/ground-truth -c init.txt; mv output.txt results3/output_$file.txt; done

