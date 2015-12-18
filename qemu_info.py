#!/usr/bin/python
# -*- coding: utf8 -*-

import os
import time
import datetime

#filename = raw_input("Enter your file name: ")
filename = "a.log"

_DEFAULT_LOG_DATE_FORMAT = "%Y-%m-%d %H:%M:%S"
qemu_img="env LC_ALL=C LANG=C qemu-img info"
qemu_result="Result was "

def get_time(line):
    str1 = line.split('.')
    time=str1[0]
    time=datetime.datetime.strptime(time, _DEFAULT_LOG_DATE_FORMAT)
    return time

file = open(filename)
start = None
standard = datetime.timedelta(seconds=40)
for line in file:
    if qemu_img in line:
        start = get_time(line)
    if qemu_result in line:
        end = get_time(line)
        if start:
            delta = end - start
            print delta
        if delta > standard:
            print "delta %s: line %s " % (delta, line)
print "finished"
file.close()
