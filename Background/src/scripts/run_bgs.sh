#!/bin/bash

MAIN_PATH="/Users/jsepulve"

# Path to videos
SEQ_PATH="${MAIN_PATH}/Activity-JPGfiles"

# Place to save mask results 
MASK_PATH="${MAIN_PATH}/BGS/build/results/masks"

# Algorithm name: 'sagmm' 'mog2' 'ucv'
ALGORITHM_NAME="sagmm"

# Activities
ACTIONS="Kick Punch RunStop ShotGunCollapse WalkTurnBack"
ACTORS="Person1 Person4"
CAMERAS="Camera_3 Camera_4"
# end video definition



# Binary command: 'bgs', 'bgs_framework', 'testUCV'
#Type of method: 1:linear 2:staircase 3:gmm normal
#cmd="./bin/testUCV"
#ext_args="--function=2"
cmd="./bin/bgs"
ext_args="--show=false"

###### From this point nothing should change .. 

# Ground-truth frames 
GT_Kick_Person1_Camera_3="2370 2911"
GT_Kick_Person1_Camera_4="2370 2911"
GT_Kick_Person4_Camera_3="200 628"
GT_Kick_Person4_Camera_4="200 628"
GT_Punch_Person1_Camera_3="2140 2607"
GT_Punch_Person1_Camera_4="2140 2607"
GT_Punch_Person4_Camera_3="92 536"
GT_Punch_Person4_Camera_4="92 536"
GT_RunStop_Person1_Camera_3="980 1418"
GT_RunStop_Person1_Camera_4="980 1418"
GT_RunStop_Person4_Camera_3="293 618"
GT_RunStop_Person4_Camera_4="293 618"
GT_ShotGunCollapse_Person1_Camera_3="267 1104"
GT_ShotGunCollapse_Person1_Camera_4="267 1104"
GT_ShotGunCollapse_Person4_Camera_3="319 1208"
GT_ShotGunCollapse_Person4_Camera_4="319 1208"
GT_WalkTurnBack_Person1_Camera_3="216 682"
GT_WalkTurnBack_Person1_Camera_4="216 682"
GT_WalkTurnBack_Person4_Camera_3="207 672"
GT_WalkTurnBack_Person4_Camera_4="207 672"
#



#config file
mask_dir="${ALGORITHM_NAME}_mask"
xmlfile="${ALGORITHM_NAME}.xml"
config="config/${xmlfile}"

# fixed parameter
_header_tag="opencv_storage"

# input parameters
_name=`echo ${ALGORITHM_NAME} | tr '[:lower:]' '[:upper:]'`
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

# Verify of 'Gen' is less than 'Range', if algorithm is 'sagmm'.
verify_sagmm_gen_value() {
    if [ "$ALGORITHM_NAME" == "sagmm" ]; then

        gen_tag="Gen"

        G=9.0 
        result=`echo $1'<'$G | bc -l`
        if [ $result -gt 0 ]; then
            gen_val=${1/.*}
        else
            gen_val=${G}
        fi

        cat ${config} | grep -v "/${_header_tag}\|${gen_tag}"             > config.tmp
        echo -e "<${gen_tag}>${gen_val}</${gen_tag}>\n</${_header_tag}>" >> config.tmp
        mv config.tmp ${config}
    fi
}

# Function definition
process_list() {
    for in2 in ${_list_2}
    do
        echo "Processing ${name} ${_tag_1}:${in1} ${_tag_2}:${in2}"

        # Verify value of 'Gen' if less than 'Range'
        verify_sagmm_gen_value $in2

        cat ${config} | grep -v "/${_header_tag}\|${_tag_2}"        > config.tmp
        echo -e "<${_tag_2}>${in2}</${_tag_2}>\n</${_header_tag}>" >> config.tmp
        mv config.tmp ${config}

        $cmd $args
        sleep 0.1
    done
}
#

set_ground_truth_frames() {

    eval FRAMES_VAL='$GT_'${name}

    init_gt=`echo ${FRAMES_VAL} | awk '{print $1}'`
    end_gt=`echo  ${FRAMES_VAL} | awk '{print $2}'`

    cat ${config} | grep -v "/${_header_tag}\|InitFGMaskFrame\|EndFGMaskFrame" > config.tmp
    echo -e  "<InitFGMaskFrame>${init_gt}</InitFGMaskFrame>"                  >> config.tmp
    echo -e  "<EndFGMaskFrame>${end_gt}</EndFGMaskFrame>\n</${_header_tag}>"  >> config.tmp

    mv config.tmp ${config}

    framework="config/Framework.xml"
    cat ${framework} | grep -v "/${_header_tag}\|InitFGMaskFrame\|EndFGMaskFrame" > Framework.tmp
    echo -e  "<InitFGMaskFrame>${init_gt}</InitFGMaskFrame>"                     >> Framework.tmp
    echo -e  "<EndFGMaskFrame>${end_gt}</EndFGMaskFrame>\n</${_header_tag}>"     >> Framework.tmp

    mv Framework.tmp ${framework}

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

            new_dir="${MASK_PATH}/${name}/${mask_dir}"
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
