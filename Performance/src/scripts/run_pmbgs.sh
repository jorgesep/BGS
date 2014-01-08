#!/bin/bash

MAIN_PATH="/home/jsepulve"

# Path to videos
GT_PATH="${MAIN_PATH}/Ground-Truth"
MASK_PATH="${MAIN_PATH}/BGS/build/results/masks"
OUTPUT="${MAIN_PATH}/BGS/build/results/measures"

# Actions definition
#ACTIONS="Kick Punch RunStop ShotGunCollapse WalkTurnBack"
ACTIONS="Kick"
ACTORS="Person1 Person4"
CAMERAS="Camera_3 Camera_4"
# end video definition

# Algorithm name: 'sagmm' 'mog2' 'ucv'
ALGORITHM_NAME="sagmm"

# Binary command
cmd="./bin/pmbgs"
ext_args="-l -f"

###### From this point nothing might be changed ..


#config file
mask_dir="${ALGORITHM_NAME}_mask"



# Loop for each action
for action in ${ACTIONS}
do
    for actor in ${ACTORS}
    do
        for cam in ${CAMERAS}
        do
            name="${action}_${actor}_${cam}" 
            # temporary work around
            #if [ "$name" == "Kick_Person1_Camera_3" ]; then
            #    continue  ### resumes iteration of an enclosing for loop ###
            #fi

            # set ground truth directory
            camera=`echo ${cam} | sed s/_//`
            ground_truth="${GT_PATH}/${action}${actor}${camera}"

            # set output directory
            new_dir="${OUTPUT}/${name}/${mask_dir}"
            if [ ! -d "${new_dir}" ]; then
                mkdir -p ${new_dir}
            else
                if [ -L "${mask_dir}" ]; then
                    unlink ${mask_dir}
                fi
            fi
            ln -s ${new_dir} ${mask_dir}

            mask="${MASK_PATH}/${name}/${mask_dir}"
            list_mask=`ls ${mask} | sort -n`

            echo $name 
            echo $ground_truth
            echo $new_dir
            echo $mask


            for i in ${list_mask}
            do
                input="${mask}/${i}"
                args="-i ${input} -g ${ground_truth} ${ext_args}"
                $cmd $args
                mv output_*.txt ${mask_dir}
            done

            unlink ${mask_dir}

        done
    done
done
