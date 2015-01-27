#!/bin/sh
result=`./ssd_stat -s | grep "Total:" | awk -F ' ' '{print $5}'`
echo ${result}
