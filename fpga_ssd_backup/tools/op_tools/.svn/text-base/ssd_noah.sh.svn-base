#/bin/sh

if [ $# -lt 2 ]
then
    echo "-1"
    exit -1
fi

#channel [0, 43], 44 means TOTAL
channel=$1
#type "read", "write", "delete", "free", "used"
type=$2

if [ ${channel} -eq 44 ]
then
    all="-s"
else
    all="-c ${channel}"
fi

#Channel READ(MB/s) WRITE(MB/s) DELETE(MB/s) FREE(MB) USED(%)
if [ ${type} == "read" ]
then
    value=`ssd_stat -i 2 ${all} | awk 'END{print $2}'`
elif [ ${type} == "write" ]
then
    value=`ssd_stat -i 2 ${all} | awk 'END{print $3}'`
elif [ ${type} == "delete" ]
then
    value=`ssd_stat -i 2 ${all} | awk 'END{print $4}'`
elif [ ${type} == "free" ]
then
    value=`ssd_stat -i 1 ${all} | awk 'END{print $5}'`
elif [ ${type} == "used" ]
then
    value=`ssd_stat -i 1 ${all} | awk 'END{print $6}'`
elif [ ${type} == "badblock" ]
then
    value=`python ssd_noah_badblock.py`
fi

echo ${value}
