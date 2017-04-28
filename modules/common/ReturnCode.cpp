#include "ReturnCode.h"

const int ReturnInfo::SUCCESS_CODE = 0;

const std::string ReturnInfo::SUCCESS_INFO = "ok";

const int ReturnInfo::FAILED_CODE = 1;

const std::string ReturnInfo::FAILED_INFO = "failed";

const int ReturnInfo::USERNAME_EXISTED_USER = 1000;                //用户名已存在
const int ReturnInfo::USERNAME_NOT_EXISTED_USER = 1001;            //用户名不存在
const int ReturnInfo::USERID_NOT_EXISTED_USER = 1002;              //用户ID不存在
const int ReturnInfo::PASSWORD_INVALID_USER = 1003;                //用户密码不正确
const int ReturnInfo::USERNAME_OR_PASSWORD_INVALID_USER = 1004;    //用户名或者密码不正确
const int ReturnInfo::EMAIL_NOT_MATCHED_USER = 1005;               //邮箱与用户名不匹配
const int ReturnInfo::DEVICE_IS_ADDED_USER = 1006;                 //设备已经被添加
const int ReturnInfo::DEVICE_NOT_BELONG_TO_USER = 1007;            //设备不归属于用户
const int ReturnInfo::DEVICE_NOT_RECORDED_USER = 1008;             //设备未记录

const int ReturnInfo::DEVICE_DOMAIN_USED_DEV = 2000;               //设备二级域名已被使用
const int ReturnInfo::DEVICE_P2PID_USED_DEV = 2001;                //设备P2PID已被使用
//未使用，占位
const int ReturnInfo::DEVICEID_NOT_EXISTED_DEV = 2998;             //设备ID不存在
const int ReturnInfo::PASSWORD_INVALID_DEV = 2999;                 //设备密码不正确

const int ReturnInfo::INPUT_PARAMETER_TOO_MUCH = 998;              //输入参数过多
const int ReturnInfo::INPUT_PARAMETER_TOO_LESS = 999;              //输入参数不足

boost::thread_specific_ptr<int> ReturnInfo::ms_iRetCode;

int ReturnInfo::RetCode()
{
    if (NULL == ms_iRetCode.get())
    {
        ms_iRetCode.reset(new int(FAILED_CODE));
    }

    return *ms_iRetCode;
}

void ReturnInfo::RetCode(const int iRetCode)
{
    if (NULL == ms_iRetCode.get())
    {
        ms_iRetCode.reset(new int(FAILED_CODE));
    }

    (*ms_iRetCode) = iRetCode;
}




