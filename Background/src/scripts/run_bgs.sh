#!/bin/bash

# Path to videos
SEQ_PATH="/media/MuHAVI_DATASET/MuHAVI"
ACTIONS="Kick Punch RunStop ShotGunCollapse WalkTurnBack"
ACTORS="Person1 Person4"
CAMERAS="Camera_3 Camera_4"
# end video definition

#config file
xmlfile="sagmm.xml"
config="config/${xmlfile}"
output_dir="sagmm_mask"
results="/media/MuHAVI_DATASET/MuHAVI/results" 

# fixed parameter
_header_tag="opencv_storage"
_tag_="Cf"
_value=`cat config/sagmm.xml | grep ${_tag_} |sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`

# Binary command
cmd="./bin/bgs"

# input parameters
loop1="config/SAGMM_Alpha.txt"
loop2="config/SAGMM_Range.txt"

# name of parameters
_tag_1=`head -1 $loop1 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`
_tag_2=`head -1 $loop2 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\1|p'`

# list of values
_list_1=`cat $loop1 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`
_list_2=`cat $loop2 | sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`

# one time loop 1
#_list_1='0.001'
#_list_2='10'

# Function definition
process_list() {
    for in2 in ${_list_2}
    do
        echo "Processing ${name} ${_tag_}:${_value} ${_tag_1}:${in1} ${_tag_2}:${in2}"
        cat ${config} | grep -v "/${_header_tag}\|${_tag_2}"        > config.tmp
        echo -e "<${_tag_2}>${in2}</${_tag_2}>\n</${_header_tag}>" >> config.tmp
        mv config.tmp ${config}
        $cmd $args
    done
}
#



# Loop for each action
for action in ${ACTIONS}
do
    for actor in ${ACTORS}
    do
        for cam in ${CAMERAS}
        do
            name="${action}_${actor}_${cam}" 

            new_dir="${results}/${name}"
            mkdir ${new_dir}
            ln -s ${new_dir} ${output_dir}

            sequence="${SEQ_PATH}/${action}/${actor}/${cam}"
            args="-i $sequence"

            for in1 in ${_list_1}
            do
                cat ${config} | grep -v "/${_header_tag}\|${_tag_1}"        > config.tmp
                echo -e "<${_tag_1}>${in1}</${_tag_1}>\n</${_header_tag}>" >> config.tmp
                mv ${config} config.bak
                mv config.tmp ${config}
                process_list
            done

            unlink ${output_dir}

        done
    done
done
