#!/bin/bash
current_path=`pwd`
ps -ef | grep product.cgi | awk '{print $2, $8}' | while read line
do
	#echo "==${line}"
	if [[ $line == *$current_path* ]]
	then
  		#echo "included"
		id=`echo "${line}" | awk '{print $1}'`
		echo "kill product_http id is "$id
		kill -9 $id
	#else
  		#echo "not included"
	fi

done
