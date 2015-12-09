#!/bin/sh
for i in `seq 80`
do
    test='compute'
    host=${test}${i}
    python rabbit_publish_nodes.py $host >> /dev/null 2>&1 &
    python rabbit_publish_nodes.py $host >> /dev/null 2>&1 &
    python rabbit_publish_nodes.py $host >> /dev/null 2>&1 &
done
