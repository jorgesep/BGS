#!/bin/bash

# Path to videos
GT_PATH="/Volumes/MuHAVi_HD/Datasets/Ground-Truth"
MASK_PATH="/Volumes/MuHAVi_HD/results"
OUTPUT="/Volumes/MuHAVi_HD/results/output"

# Actions definition
ACTIONS="Kick Punch RunStop ShotGunCollapse WalkTurnBack"
ACTORS="Person1 Person4"
CAMERAS="Camera_3 Camera_4"
# end video definition

# Algorithm name: 'sagmm' 'mog2' 'ucv'
ALGORITHM_NAME="mog2"

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

            # set ground truth directory
            ground_truth="${GT_PATH}/${action}${actor}${cam}"

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
