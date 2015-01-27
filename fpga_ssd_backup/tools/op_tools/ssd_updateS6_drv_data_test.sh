#!/bin/sh
# Filename : ssd_updateS6_drv_data_test.sh
# Function : 1) load the s6 and driver with wget;
#             2) test for the process -> SSD-open & write & read each channel & close 
#                when open failed, S6 logic will be reload till success or arrive 10 times
# 2013-10-30 created by zhangwei<zhangwei29@baidu.com>
# Copyright  (c)  2001 baidu.com


rm -rf ssd_drv.ko
echo "remove ssd_drv.ko successfully "
wget http://ikernel.baidu.com/ssd/temp_test/ssd_drv.ko
echo "get newest ssd_drv.ko successfully!"

rmmod  ssd_drv.ko
sleep 5s 
insmod ssd_drv.ko
sleep 5s
echo "change the driver successfully!"

for ((cnt=0; cnt<10; ++cnt))
do 
    ./open
    if [ $? -ne 0 ] && [ $? -ne -25 ];
    then  
         ./wr_n 0x80f8 0 && ./wr_n 0x80f8 1 && ./wr_n 0x80e8 1 && ./cfg_s6_n_40 1 && ./wr_n 0x80e8 1
    else
         break 1
    fi
done

if [ ${cnt} -ge 10 ];
then 
    echo " load s6 failed!"
    exit -1
fi

echo "load S6 sucessfully in cycle :" ${cnt}
./close_force

echo "begin the wr & rd test"
id_num="1"  # id num write to each channel per cycle

for ((cnt=0; cnt<2; ++cnt))
do
    ./ssd_check_all ${id_num}
    if [ $? -ne 0 ];
    then
       echo "stress test failed"
       exit -1
    fi
    echo "cycle is ${cnt}"
done
echo "stress test of wr & rd successfully"
