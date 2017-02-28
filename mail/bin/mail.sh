#!/bin/bash
if [ $# -lt 3 ]
then
    echo "usage: $0 recipient username pwd";
    exit;
fi

Recipient=$1;
UserName=$2;
Pwd=$3

./mailsender 'smtp.qiye.163.com' 2 'ringviews@annigroup.com' 'anni@2688' 'RingViews' 'ringviews@annigroup.com' $Recipient 'RingViews' '<success><b><br>感谢您注册RingViews，请记住您的用户账号：$UserName 登陆密码：$Pwd<br></b></success>  <success><b><br>Thank you for registering RingViews, please remember your user account: $UserName password: $Pwd<br></b></success>'

