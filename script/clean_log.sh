#!/bin/bash
if [ $# -lt 2 ]
then
    echo "usage: $0 path size";
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
        dir="$1""/*"
        rm -fr $dir
        echo "end clean $1"  
    fi  
done

#[root@iZrj9d3k592pvkazrcdxjiZ log]# ps -ef | grep clean_log.sh 
#root      3286     1  0 May19 ?        08:20:48 /bin/bash ./clean_log.sh /root/IOTC_Server/log/mt/ANN_AA11_0001_0001_5 500
