#!/bin/bash

# Path to videos
AVI="WalkTurnBack-Camera_3-Person1.avi"
GROUNDT="jpeg/Person1/ground-truth"
VIDEOPATH="/Users/jsepulve/Tesis/Videos"
VIDEO="$VIDEOPATH/$AVI"
THRUTH="$VIDEOPATH/$GROUNDT"
# end video definition

# Function definition
process_list() {
    _para=`head -1 config/$file | awk '{print $1}'`
    _list=`cat config/$file     | awk '{print $3}'`
    for i in $_list
    do
        echo "Processing $_para : $i"
        cat config/init.txt | grep -v $_para > init.txt
        echo "$_para : $i" >> init.txt
        $cmd $args
        mv output.txt $DST/output_$range_$i.txt
    done
}
#

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

#LIST_FILES="Range.txt Alpha.txt"
#LIST_FILES="Alpha.txt"
LIST_FILES="Range.txt"
rnumbers=`cat config/Range.txt | awk '{print $3}'`

for range in $rnumbers
do
    cat config/init.txt | grep -v Range > init.tmp
    echo "Range : $range" >> init.tmp
    mv init.tmp config/init.txt
    file="Alpha.txt"
    process_list
done




#cat Alpha.txt | while read in; do date; file=`echo $in | awk '{print $3}'`; echo output_$file.txt; cat init.txt | grep -v Alfa > init.tmp; echo $in >> init.tmp; mv init.tmp init.txt; ./bin/bgs -i /Users/jsepulve/Tesis/Videos/WalkTurnBack-Camera_3-Person1.avi -g /Users/jsepulve/Tesis/Code/Matlab/MuHavi/WalkTurnBack/Person1/ground-truth -c init.txt; mv output.txt results3/output_$file.txt; done

