#!/bin/sh

if [ $# -lt 1 ]
then
    echo " sh auto_mark_bad_block.sh <ssd_check.log>"
    exit -1
fi

err_log=$1
count=`awk '{if($0 ~/ssd read error/) print $(NF - 1)}' ${err_log} | wc -l`
IDS=`awk '{if($0 ~/ssd read error/) print $(NF - 1)}' ${err_log}`
max_bad_blocks=8000

if [ ${count} -gt ${max_bad_blocks} ]
then
    echo "too many bad blocks: "${count}
else
    /home/op_tools/open >/dev/null 2>/dev/null
    for ID in ${IDS}
    do
        echo "mark bad ID: "${ID}
        ssd_mark_bad_block -i ${ID}
    done
    /home/op_tools/close_force
fi
