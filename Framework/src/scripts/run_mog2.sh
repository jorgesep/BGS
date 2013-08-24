#!/bin/bash

# Path to videos
#SEQ_PATH="/media/MuHAVI_DATASET/MuHAVI"
SEQ_PATH="/Volumes/MuHAVi_HD/KUVideoData/Reason/Activity-JPGfiles"
#ACTIONS="Kick Punch RunStop ShotGunCollapse WalkTurnBack"
ACTIONS="WalkTurnBack"
ACTORS="Person1"
#ACTORS="Person1 Person4"
CAMERAS="Camera_3"
#CAMERAS="Camera_3 Camera_4"
# end video definition

#config file
xmlfile="mog2.xml"
config="config/${xmlfile}"
mask_dir="mog2_mask"
#results="/media/MuHAVI_DATASET/MuHAVI/results" 
results="/Users/jsepulve/Downloads/BGS/build/results"

# fixed parameter
_header_tag="opencv_storage"
_tag_="History"
_value=`cat ${config} | grep ${_tag_} |sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`

# Binary command
cmd="./bin/bgs_framework"

# input parameters
loop1="config/MOG2_LearningRate.txt"
loop2="config/MOG2_Threshold.txt"

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
        echo "Processing ${name} ${_tag_1}:${in1} ${_tag_2}:${in2}"
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

            new_dir="${results}/${name}/${mask_dir}"
            if [ ! -d "${new_dir}" ]; then
                mkdir -p ${new_dir}
            else 
                if [ -L "${mask_dir}" ]; then 
                    unlink ${mask_dir}
                fi
            fi
            ln -s ${new_dir} ${mask_dir}

            sequence="${SEQ_PATH}/${action}/${actor}/${cam}"
            args="-i $sequence --show=false"

            for in1 in ${_list_1}
            do
                cat ${config} | grep -v "/${_header_tag}\|${_tag_1}"        > config.tmp
                echo -e "<${_tag_1}>${in1}</${_tag_1}>\n</${_header_tag}>" >> config.tmp
                mv ${config} config.bak
                mv config.tmp ${config}
                process_list
            done

            unlink ${mask_dir}

        done
    done
done
