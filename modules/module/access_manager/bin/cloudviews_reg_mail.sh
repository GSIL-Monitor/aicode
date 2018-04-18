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

msg1="<success><b><br>Thank you for registering CloudViews, please remember your user account: "
msg2="$msg1""$UserName"" password: "
msg3="$msg2""$Pwd""<br></b></success>  <success><b><br>感谢您注册CloudViews，请记住您的用户账号："
msg4="$msg3""$UserName"" 登陆密码：""$Pwd"
msg5="$msg4""<br></b></success>"
./mailsender 'smtp.163.com' 2 25 'cloudviews@163.com' 'cloudviews163' 'CloudViews' 'cloudviews@163.com' $Recipient 'CloudViews' "$msg5"

