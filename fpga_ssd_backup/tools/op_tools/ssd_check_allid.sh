#!/bin/sh

# Filename : ssd_check_allid.sh
# Function : check all id data with crc
#       
# 2013-10-23 created by zhangwei<zhangwei29@baidu.com>
# Copyright  (c)  2001 baidu.com

./open
./ssd_ls > all_id_tmp.log
for line in $(cat all_id_tmp.log)
do
    ./ssd_read_id ${line} 0 temp_id_data
    if [ $? -eq 0 ]
    then 
        ./ssd_read_id ${line} 1 ${line} | grep "error"
        if [ $? -eq 0 ]
        then
            echo "crc check error in ID : ${line}"
        fi
        rm -rf ${line}
	else
		echo ${line} >> right_id.log
    fi
done
rm -rf temp_id_data
rm -rf all_id_tmp.log
./close_force

