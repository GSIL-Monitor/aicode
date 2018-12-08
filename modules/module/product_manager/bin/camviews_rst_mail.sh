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

msg1="<success><b><br>你的应用临时登录密码是："
msg3="$msg1""$Pwd""，请重置新密码在你的应用程序设置"
msg4="$msg3""<br></b></success>   <success><b><br>Hello,your APP temporary login password is ""$Pwd"",Please reset your new password in your APP setting"
msg5="$msg4""<br></b></success>"
./mailsender 'smtp.qiye.163.com' 2 'camviews@annigroup.com' 'Anni2688' 'CamViews' 'camviews@annigroup.com' $Recipient 'CamViews' "$msg5"

