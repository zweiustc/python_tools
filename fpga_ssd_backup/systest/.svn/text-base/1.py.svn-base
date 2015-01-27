#!/usr/bin/python
# Filename : insmod.py for ssd module test
# Author   : Fanchao  hi: fan_chao_00
# Copyright@baidu.com

import os
import commands

ssd_path   = '/home/op_tools/ssd_drv.ko'
ssd_module = "ssd_drv"
ret        = 0;

##lsmod ssd module
if commands.getoutput("lsmod |grep ^\%s\ | awk '{print $1}'" % ssd_module) == ssd_module:
        print "lsmod %s module successfully" % ssd_module
        ret = -1
else:
        print "ERROR! can not lsmod %s, it can not insmod" % ssd_module
        ret = -2

#rmmod ssd module
if ret == -1:
	if not os.system("rmmod %s" % ssd_module):
        	print "rmmod %s module successfully" % ssd_module
	else:
		print "ERROR! can not rmmod %d module" % ssd_module
else:
	print "% module is not exist"% ssd_module


#insmod ssd module
if  not os.system("insmod %s" % ssd_path):
        print "insmod %s module successfully" % ssd_module
else:
        print "ERROR! can not insmod %s" % ssd_module
        ret = -1


