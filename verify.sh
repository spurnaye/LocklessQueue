#!/bin/bash


for i in `seq 1 $1`;
do
    comm="egrep -c '^$i,' file"
    #echo $comm
    v=`eval $comm`
    #echo $v
    if [ "$v" -ne "2" ]; then
        echo "Error for number $i"
    fi
done
