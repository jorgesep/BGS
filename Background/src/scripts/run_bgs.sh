#!/bin/bash

# Path to videos
PATH_VIDEOS="/Users/jsepulve/Tesis/Videos"
FILE="WalkTurnBack-Camera_3-Person1.avi"
DIR="ground-truth"
VIDEO="$PATH_VIDEOS/$FILE"
TRUTH="$PATH_VIDEOS/jpeg/Person1/$DIR"
# end video definition

#config file
xmlfile="sagmm.xml"
config="config/${xmlfile}"

# fixed parameter
_header_tag="opencv_storage"
_tag_="Cf"
_value=`cat config/sagmm.xml | grep ${_tag_} |sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`
OUTPUT="results/${_tag_}_${_value}"

if [ ! -d "${OUTPUT}" ]; then
        mkdir -p ${OUTPUT}
fi


#DATE=`date "+%Y-%m-%d_%H:%M:%S"`
#RESULTS="results/`date "+%Y-%m-%d_%H-%M-%S"`"
#if [ ! -d "$RESULTS" ]; then
#        mkdir -p $RESULTS
#fi

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
    if [ -d $TRUTH ]; then
        args="-i $INPUT -g $TRUTH"
    else
        args="-i $INPUT"
    fi
else
    args="-i $INPUT -g $GT"
fi

cmd="./bin/bgs"

loop1="config/Alpha.txt"
loop2="config/Range.txt"

_tag_1=`head -1 $loop1 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`
_tag_2=`head -1 $loop2 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`

_list_1=`cat $loop1 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`
_list_2=`cat $loop2 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`

# one time loop 1
_list_1='0.001'
_list_2='10'



# Function definition
process_list() {
    for in2 in ${_list_2}
    do
        echo "Processing ${_tag_}:${_value} ${_tag_1}:${in1} ${_tag_2}:${in2}"
        cat ${config} | grep -v "/${_header_tag}\|${_tag_2}"        > config.tmp
        echo -e "<${_tag_2}>${in2}</${_tag_2}>\n</${_header_tag}>" >> config.tmp
        mv config.tmp ${config}
        $cmd $args
        mv output.txt ${OUTPUT}/output_${_tag_1}_${in1}_${_tag_2}_${in2}.txt
    done
}
#


for in1 in ${_list_1}
do
    cat ${config} | grep -v "/${_header_tag}\|${_tag_1}"        > config.tmp
    echo -e "<${_tag_1}>${in1}</${_tag_1}>\n</${_header_tag}>" >> config.tmp
    mv ${config} config.bak
    mv config.tmp ${config}
    process_list
    #mv config.bak ${config}
done

