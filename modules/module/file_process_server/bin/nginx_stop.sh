#!/bin/bash
current_path=`pwd`
ps -ef | grep nginx | awk '{print $2, $13}' | while read line
do
	#echo "==${line}"
	if [[ $line == *$current_path* ]]
	then
  		#echo "included"
		id=`echo "${line}" | awk '{print $1}'`
		echo "kill nginx id is "$id
		kill -TERM $id
	#else
  		#echo "not included"
	fi

done
