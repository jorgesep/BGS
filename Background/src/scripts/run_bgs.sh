#!/bin/bash

# Path to videos
AVI="WalkTurnBack-Camera_3-Person1.avi"
GROUNDT="ground-truth"
VIDEOPATH="/Users/jsepulve/Tesis/Videos"
VIDEO="$VIDEOPATH/$AVI"
THRUTH="$VIDEOPATH/$GROUNDT"
# end video definition


DATE=`date "+%Y-%m-%d_%H:%M:%S"`
DST="results/`date "+%Y-%m-%d_%H-%M-%S"`"

if [ ! -d "$DST" ]; then
    mkdir -p $DST
fi

INPUT=$1
GT=$2

if [ "$INPUT" == "" ]; then
    if [ -f $VIDEO ]; then
        INPUT="$VIDEO"
    else
        echo "Inset input video to be processed. "
        exit
    fi
fi

if [ "$GT" == "" ]; then
    if [ -d $THRUTH ]; then
        args="-i $INPUT -g $THRUTH -c init.txt"
    else
        args="-i $INPUT -c init.txt"
    fi
else
    args="-i $INPUT -g $GT -c init.txt"
fi

cmd="./bin/bgs"

loop1="config/Alpha.txt"
loop2="config/Range.txt"
_list_1=`cat $loop1 | awk '{print $3}'`
_list_2=`cat $loop2 | awk '{print $3}'`

_param_1=`head -1 $loop1 | awk '{print $1}'`
_param_2=`head -1 $loop2 | awk '{print $1}'`

# one time loop 1
_list_1='0.001'

# Function definition
process_list() {
    for in2 in ${_list_2}
    do
        echo "Processing ${_param_1} : ${in1} ${in2}"
        cat init.txt | grep -v ${_param_2} > init.tmp
        echo "${_param_2} : ${in2}" >> init.tmp
        mv init.tmp init.txt
        $cmd $args
        mv output.txt $DST/output_${in1}_${in2}.txt
    done
}
#


for in1 in ${_list_1}
do
    cat config/init.txt | grep -v ${_param_1} > init.tmp
    echo "${_param_1} : ${in1}" >> init.tmp
    mv init.tmp init.txt
    process_list
done




#cat Alpha.txt | while read in; do date; file=`echo $in | awk '{print $3}'`; echo output_$file.txt; cat init.txt | grep -v Alfa > init.tmp; echo $in >> init.tmp; mv init.tmp init.txt; ./bin/bgs -i /Users/jsepulve/Tesis/Videos/WalkTurnBack-Camera_3-Person1.avi -g /Users/jsepulve/Tesis/Code/Matlab/MuHavi/WalkTurnBack/Person1/ground-truth -c init.txt; mv output.txt results3/output_$file.txt; done

