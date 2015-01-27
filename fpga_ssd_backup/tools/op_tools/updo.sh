#!/bin/sh
./wr_n 0x80f8 0
./wr_n 0x80f8 1
./wr_n 0x80e8 1
./close_force

for((i=0;i<10;i++))
do
    ./open
    if [ $? -ne 0 ] && [ $? -ne -25 ];
    then 
		./wr_n 0x80f8 0 && ./wr_n 0x80f8 1 && ./wr_n 0x80e8 1 && ./cfg_s6_n_40 1 && ./wr_n 0x80e8 1
	else
		./close_force
		break 1
	fi    
done
./ssd_update_check 0 43 >check.log
