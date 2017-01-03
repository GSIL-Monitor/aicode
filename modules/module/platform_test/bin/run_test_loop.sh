#!/bin/sh

host=172.16.23.186
port=6789
procnum=100
looptimes=100
file_list=('512k.file' '1m.file' '10m.file' '100m.file')

while [ 1 ]
do
	for ((k=0;k<$looptimes;k++))
	do
	  
	  setsid ./platform_test &
	  
	done
	sleep 5  
	echo "start kill task for test interrupt business..." && ps -ef|grep platform_test|grep -v grep | awk '{print $2}'|xargs kill -9
done



