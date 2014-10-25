#!/bin/bash

# Path to videos
PATH_VIDEOS="/Users/jsepulve/Tesis/Videos"
FILE="WalkTurnBack-Camera_3-Person1.avi"
DIR="ground-truth"
VIDEO="$PATH_VIDEOS/$FILE"
TRUTH="$PATH_VIDEOS/$DIR"
# end video definition

# config file
config="config/np_init.yml"

DATE=`date "+%Y-%m-%d_%H:%M:%S"`
RESULTS="results/`date "+%Y-%m-%d_%H-%M-%S"`"

if [ ! -d "$RESULTS" ]; then
    mkdir -p $RESULTS
fi


# Video directory
if [ -f $VIDEO ]; then
    args="-i $VIDEO -c config.yml"
else
    echo "Insert video to be processed... "
    exit
fi


cmd="./bin/npbgs"

#loop1="config/Alpha.txt"
#loop2="config/Threshold.txt"
loop1="config/WindowSize.txt"
loop2="config/Length.txt"
_list_1=`cat $loop1 | awk '{print $3}'`
_list_2=`cat $loop2 | awk '{print $3}'`

_param_1=`head -1 $loop1 | awk '{print $1}'`
_param_2=`head -1 $loop2 | awk '{print $1}'`

# one time loop 1
#_list_2='1.0e-07'

# Function definition
process_list() {
    for in2 in ${_list_2}
    do
    if [ "$in2" -le "$in1" ]; then
        echo "Processing ${_param_1} : ${in1} ${in2}"
        cat config.yml | grep -v ${_param_2} > init.tmp
        echo "${_param_2}: ${in2}" >> init.tmp
        mv init.tmp config.yml
        $cmd $args
        if [ -f "output.txt" ]; then
            mv output.txt $RESULTS/output_${in1}_${in2}.txt
        fi
    fi
    done
}
#


for in1 in ${_list_1}
do
    cat $config | grep -v ${_param_1} > init.tmp
    echo "${_param_1}: ${in1}" >> init.tmp
    mv init.tmp config.yml
    process_list
done


