#!/bin/sh
# Filename : ssd_update_v5.sh
# Function : 1) load the v5 with wget; 2) config the v5 to fpga
# 2013-10-30 created by zhangwei<zhangwei29@baidu.com>
# Copyright  (c)  2001 baidu.com
rm -rf fifo_final_1014.mcs
wget http://ikernel.baidu.com/ssd/temp_test/afifo_final_1014.mcs
echo "wget  afifo_final_1014.mcs successfully!"

./cfg_flash_n fifo_final_1014.mcs
./cfg_v5_n
echo "change the V5 logic mcs successfully!"
