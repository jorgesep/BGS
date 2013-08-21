#!/bin/bash

# Path to videos
PATH_VIDEOS="/data/MuHAVI/WalkTurnBack/Person1/Camera_3"
FILE="WalkTurnBack-Camera_3-Person1.avi"
DIR="ground-truth"
VIDEO="$PATH_VIDEOS"
TRUTH="$PATH_VIDEOS/jpeg/Person1/$DIR"
# end video definition

#config file
xmlfile="ucv.xml"
config="config/${xmlfile}"

# fixed parameter
_header_tag="opencv_storage"
_tag_="Alpha"
_value=`cat ${config} | grep ${_tag_} |sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`
OUTPUT="results/${_tag_}_${_value}"

# Check video directory
if [ -d ${VIDEO} ]; then
    args="-i ${VIDEO}"
else
    echo "Invalid name of video ..."
    exit
fi

cmd="./bin/testUCV"

loop1="config/UCV_LearningRate.txt"
loop2="config/UCV_Threshold.txt"

_tag_1=`head -1 $loop1 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`
_tag_2=`head -1 $loop2 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`

_list_1=`cat $loop1 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`
_list_2=`cat $loop2 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`

# one time loop 1
#_list_1='0.001'

# Function definition
process_list() {
    for in2 in ${_list_2}
    do
        echo "Processing ${_tag_}:${_value} ${_tag_1}:${in1} ${_tag_2}:${in2}"
        cat ${config} | grep -v "/${_header_tag}\|${_tag_2}"        > config.tmp
        echo -e "<${_tag_2}>${in2}</${_tag_2}>\n</${_header_tag}>" >> config.tmp
        mv config.tmp ${config}
        $cmd $args
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

