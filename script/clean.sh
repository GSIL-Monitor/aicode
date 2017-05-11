#!/bin/bash
if [ $# -lt 2 ]
then
    echo "usage: $0 filename filesize";
    exit;
fi

flag=1  
result=1  
while [ "$flag" -eq 1 ]  
do  
    sleep 1s  
    result=`du -m $1|awk '{print $1}'`
    if [ "$result" -gt $2 ]; then  
        echo "begin clean $1"
        >$1
        echo "end clean $1"  
    fi  
done
