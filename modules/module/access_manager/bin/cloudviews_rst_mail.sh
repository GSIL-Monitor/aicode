#!/bin/bash
if [ $# -lt 3 ]
then
    echo "usage: $0 recipient username pwd";
    exit;
fi

Recipient=$1;
UserName=$2;
Pwd=$3

echo $UserName
echo $Pwd

msg1="<success><b><br>Hello,your CloudViews APP temporary login password is "
msg3="$msg1""$Pwd"",Please reset your new password in your APP setting."
msg4="$msg3""<br></b></success>   <success><b><br>您的云视APP应用临时登录密码是：""$Pwd""，请在APP应用程序中重置新密码。"
msg5="$msg4""<br></b></success>"
./mailsender 'smtp.163.com' 2 25 'cloudviews@163.com' 'cloudviews163' 'CloudViews' 'cloudviews@163.com' $Recipient 'CloudViews' "$msg5"

