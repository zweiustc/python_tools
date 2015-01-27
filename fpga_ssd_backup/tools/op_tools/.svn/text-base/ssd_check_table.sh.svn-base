#!/bin/sh

if [ $# -lt 1 ]
then
	echo "arg error"
	exit -1
fi

/home/op_tools/open > /dev/null

for channel in `seq 0 43`
do
    #address table
    table=`/home/op_tools/ssd_read_table -c ${channel} -t 0 -d | awk 'BEGIN{OF='\t'; val = 0}{if(($1 ~/^[0-9]/) &&($NF == 0)) val++}END{print val}'`
    if [ ${table} -gt 20 ]
    then
        echo "${channel} addr table error"
        if [ $1 = 'close' ]
        then
            /home/op_tools/close_force > /dev/null
        fi
        exit -1
    fi

    #erase table
    table=`/home/op_tools/ssd_read_table -c ${channel} -t 1 -d | awk 'BEGIN{OF='\t'; val = 0}{if(($1 ~/^[0-9]/) &&($NF == 0)) val++}END{print val}'`
    if [ ${table} -gt 20 ]
    then
        echo "${channel} erase table error"
        if [ $1 = 'close' ]
        then
            /home/op_tools/close_force > /dev/null
        fi
        exit -1
    fi
done

echo "success"

if [ $1 = 'close' ]
then
    /home/op_tools/close_force > /dev/null
fi
exit 0
