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

msg1="<success><b><br>感谢您注册CamViews，请记住您的用户账号："
msg2="$msg1""$UserName"" 登陆密码："
msg3="$msg2""$Pwd""<br></b></success>  <success><b><br>Thank you for registering CamViews, please remember your user account: "
msg4="$msg3""$UserName"" password: ""$Pwd"
msg5="$msg4""<br></b></success>"
./mailsender 'smtp.qiye.163.com' 2 'camviews@annigroup.com' 'Anni2688' 'CamViews' 'camviews@annigroup.com' $Recipient 'CamViews' "$msg5"
