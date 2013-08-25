#!/bin/bash

# Path to videos
SEQ_PATH="/media/MuHAVI_DATASET/MuHAVI"
ACTIONS="Kick Punch RunStop ShotGunCollapse WalkTurnBack"
ACTORS="Person1 Person4"
CAMERAS="Camera_3 Camera_4"
# end video definition

# Binary command
cmd="./bin/bgs"
ext_args=""

# Ground-truth frames 
GT_Kick_Person1_Camera3="2370 2911"
GT_Kick_Person1_Camera4="2370 2911"
GT_Kick_Person4_Camera3="200 628"
GT_Kick_Person4_Camera4="200 628"
GT_Punch_Person1_Camera3="2140 2607"
GT_Punch_Person1_Camera4="2140 2607"
GT_Punch_Person4_Camera3="92 536"
GT_Punch_Person4_Camera4="92 536"
GT_RunStop_Person1_Camera3="980 1418"
GT_RunStop_Person1_Camera4="980 1418"
GT_RunStop_Person4_Camera3="293 618"
GT_RunStop_Person4_Camera4="293 618"
GT_ShotgunCollapse_Person1_Camera3="267 1104"
GT_ShotgunCollapse_Person1_Camera4="267 1104"
GT_ShotgunCollapse_Person4_Camera3="319 1208"
GT_ShotgunCollapse_Person4_Camera4="319 1208"
GT_WalkTurnBack_Person1_Camera3="216 682"
GT_WalkTurnBack_Person1_Camera4="216 682"
GT_WalkTurnBack_Person4_Camera3="207 672"
GT_WalkTurnBack_Person4_Camera4="207 672"
#



#config file
algorithm="sagmm"
mask_dir="${algorithm}_mask"
results="/media/MuHAVI_DATASET/MuHAVI/results" 
xmlfile="${algorithm}.xml"
config="config/${xmlfile}"

# fixed parameter
_header_tag="opencv_storage"
#_tag_="Cf"
#_value=`cat ${config} | grep ${_tag_} |sed -n 's|<\([a-zA-Z]*\)>\(.*\)</[a-zA-Z]*>|\2|p'`

# input parameters
_name=`echo ${algorithm} | tr '[:lower:]' '[:upper:]'`
loop1="config/${_name}_LearningRate.txt"
loop2="config/${_name}_Threshold.txt"

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

set_ground_truth_frames() {

    GT_FRAMES="GT_${name}"
    init_gt=`echo ${GT_FRAMES} | awk '{print $1}'`
    end_gt=`echo ${GT_FRAMES} | awk '{print $2}'`

    cat ${config} | grep -v "/${_header_tag}\|InitFGMaskFrame\|EndFGMaskFrame" > config.tmp
    echo -e  "<InitFGMaskFrame>${init_gt}</InitFGMaskFrame>"                  >> config.tmp
    echo -e  "<EndFGMaskFrame>${end_gt}</EndFGMaskFrame>\n</${_header_tag}>"  >> config.tmp

    mv config.tmp ${config}
}

# Loop for each action
for action in ${ACTIONS}
do
    for actor in ${ACTORS}
    do
        for cam in ${CAMERAS}
        do
            name="${action}_${actor}_${cam}" 
            set_ground_truth_frames

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
            args="-i $sequence ${ext_args}"

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
