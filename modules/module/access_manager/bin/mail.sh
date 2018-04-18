#!/bin/bash
if [ $# -lt 5 ]
then
    echo "usage: $0 recipient username pwd type action";
    exit;
fi

Recipient=$1;
UserName=$2;
Pwd=$3
Type=$4
Action=$5

#echo $UserName
#echo $Pwd

if [[ "camviews" = $Type ]]; then

    if [[ "reg" = $Action ]]; then
        ./camviews_reg_mail.sh $Recipient $UserName $Pwd
    elif [[ "rst" = $Action ]]; then
        ./camviews_rst_mail.sh $Recipient $UserName $Pwd
    else
        echo 'Error action param'
    fi
    
elif [[ "ring" = $Type ]]; then 

    if [[ "reg" = $Action ]]; then
        ./ring_reg_mail.sh $Recipient $UserName $Pwd
    elif [[ "rst" = $Action ]]; then
        ./ring_rst_mail.sh $Recipient $UserName $Pwd
    else
        echo 'Error action param'
    fi
    
elif [[ "cloudviews" = $Type ]]; then 

    if [[ "reg" = $Action ]]; then
        ./cloudviews_reg_mail.sh $Recipient $UserName $Pwd
    elif [[ "rst" = $Action ]]; then
        ./cloudviews_rst_mail.sh $Recipient $UserName $Pwd
    else
        echo 'Error action param'
    fi
else
    echo 'Error param.'
    
fi

