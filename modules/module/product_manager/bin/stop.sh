#!/bin/bash
current_path=`pwd`

ps -ef | grep product_manager | awk '{print $2, $8}' | while read pd
do
  #echo '-----------'
  #echo $pd
  #echo '-----------'
 
  #tmp=`echo "${pd}" | awk '{print $2}'`
  #echo $tmp

  a='product'
  if [[ $pd =~ $a ]]
  then
    #echo 'mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm: '$pd
    
    pidkill=`echo "${pd}" | awk '{print $1}'`
    #echo $pidkill
    
    ls -l /proc/$pidkill | awk '{print $11}' | while read ln
    do
      #echo '++++++'
      #echo $ln
      if [[ $ln =~ $current_path ]]
      then
        #echo $pidkill
        echo "kill product_manager id is "$pidkill
        kill -9 $pidkill
        break
      fi
    done
  fi  

done


