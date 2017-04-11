#!/bin/bash
current_path=`pwd`
current_path=$current_path'/filemgr.cgi'
#echo $current_path
ps -ef | grep filemgr.cgi | awk '{print $2, $8}' | while read line
do
	#echo "==${line}"
	if [[ $line == *$current_path* ]]
	then
  		#echo "included"
		id=`echo "${line}" | awk '{print $1}'`
		echo "kill filemgr id is "$id
		kill -9 $id
	#else
  		#echo "not included"
	fi

done
