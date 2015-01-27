#! /bin/bash
if [ -d "output" ];then
	rm -rf output
	if [ $? -ne 0 ];then
		echo "Error: Remove output dir failed!"
		exit 1
	fi
fi

mkdir output

cp -f ssd_drv.ko output/
rm -f *.o *.ko *.mod.c *.order *.symvers
