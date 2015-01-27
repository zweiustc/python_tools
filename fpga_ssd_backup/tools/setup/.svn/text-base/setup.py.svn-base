#!/usr/bin/python

import os 
import shutil
from distutils.core import setup
from distutils import sysconfig

setup(name='ssd_op',
        version='0.2',
        description='ssd operation tools',
        packages=['ssd_op'])

#init ssdServer
SSD_INIT_DIR = '/etc/rc.d/init.d'
SSD_INIT_NAME = 'ssdServer'
SSd_INIT_LEVEL = '345'

initPath = os.path.join(SSD_INIT_DIR, SSD_INIT_NAME)
if os.path.exists(initPath):
    os.remove(initPath)

SSD_LIB_PATH = sysconfig.get_python_lib()
SSD_BIN_PATH = os.path.join(SSD_LIB_PATH, 'ssd_op/ssdRpcServer.py')
SSD_BIN_PATH_E = SSD_BIN_PATH.replace('/', '\/')

srcPath = os.path.join(os.getcwd(), 'setup', SSD_INIT_NAME)
shutil.copy(srcPath, SSD_INIT_DIR)
cmd = 'sed -i \'s/SERVER=/SERVER=\"' + SSD_BIN_PATH_E + '\"/g\' ' + initPath
os.popen(cmd)

#chkconfig
cmd = 'chkconfig --add ' + SSD_INIT_NAME
os.popen(cmd)
cmd = 'chkconfig --level ' + SSd_INIT_LEVEL + ' ' + SSD_INIT_NAME + ' on'
os.popen(cmd)

#modify rc.local
cmd = 'sed -i \'/service ssdServer restart/d\' /etc/rc.d/rc.local'
os.popen(cmd)
cmd = 'echo service ssdServer start >> /etc/rc.d/rc.local'
os.popen(cmd)

dirName = os.path.dirname(SSD_BIN_PATH)
if os.path.exists(dirName):
	shutil.rmtree(dirName)
dir = os.path.join(os.getcwd(), 'ssd_op')
shutil.copytree(dir, dirName)
