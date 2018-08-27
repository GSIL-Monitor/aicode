#!/bin/bash
if [ $# -lt 3 ]
then
    echo "usage: $0 recipient username pwd";
    exit;
fi

date=`date +%Y%m%d-%H:%M:%S`
msg_title="Belling-""$date"
echo $msg_title

Recipient=$1;
UserName=$2;
Pwd=$3

echo $UserName
echo $Pwd

msg1="<success><b><br>Thank you for registering Belling, please remember your user account: "
msg2="$msg1""$UserName"
msg3="$msg2""<br></b></success>  <success><b><br>感谢您注册Belling，请记住您的用户账号："
msg4="$msg3""$UserName"
msg5="$msg4""<br></b></success>  <success><b><br>""$msg_title""<br></b></success>"
./mailsender 'smtp.163.com' 2 25 'bellingservice@163.com' 'belling163' 'Belling' 'bellingservice@163.com' $Recipient "$msg_title" "$msg5"

