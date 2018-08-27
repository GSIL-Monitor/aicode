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
const int ReturnInfo::DEVICE_P2PID_NOT_RECORDED_USER = 1009;       //设备P2PID未记录
const int ReturnInfo::DEVICE_ADDED_BY_CURRENT_USER = 1010;         //当前用户已添加此设备
const int ReturnInfo::CLOUD_STORAGE_NOT_PAID_USER = 1011;          //用户未开通付费云存储服务
const int ReturnInfo::UNDELETED_DEVICE_EXISTED_USER = 1012;        //用户仍有未删除的设备
const int ReturnInfo::ACCOUNT_LOGIN_AT_OTHER_TERMINAL = 1013;      //账户在其他终端登录
const int ReturnInfo::SHARED_DEVICE_NOT_ADDED_BY_USER = 1014;      //设备尚未被用户添加
const int ReturnInfo::SHARED_USERNAME_IS_INVALID_USER = 1015;      //用户名有误，找不到此用户
const int ReturnInfo::LOGIN_USING_TEMP_PASSWORD_USER = 1016;       //用户使用临时密码登录，必须修改密码
const int ReturnInfo::SHARED_USERID_IS_INVALID_USER = 1017;        //用户ID有误，找不到此用户

const int ReturnInfo::SHARING_DEVICE_UP_TO_LIMIT = 3001;

const int ReturnInfo::DEVICE_DOMAIN_USED_DEV = 2000;               //设备二级域名已被使用
const int ReturnInfo::DEVICE_P2PID_USED_DEV = 2001;                //设备P2PID已被使用
const int ReturnInfo::DEVICE_DOMAINNAME_INVALID = 2002;            //设备二级域名不可用
const int ReturnInfo::DEVICE_NOT_ADDED_BY_USER = 2003;             //设备尚未被用户添加
//未使用，占位
const int ReturnInfo::DEVICEID_NOT_EXISTED_DEV = 2998;             //设备ID不存在
const int ReturnInfo::PASSWORD_INVALID_DEV = 2999;                 //设备密码不正确

const int ReturnInfo::SESSION_TIMEOUT = 996;                       //会话超时
const int ReturnInfo::NO_ACCESS_PERMISSION = 997;                  //没有访问权限
const int ReturnInfo::INPUT_PARAMETER_TOO_MUCH = 998;              //输入参数过多
const int ReturnInfo::INPUT_PARAMETER_TOO_LESS = 999;              //输入参数不足

const int ReturnInfo::SENSOR_TYPE_DUPLICATE = 3000;

const int ReturnInfo::CMS_P2PID_DUPLICATE = 3100;

const int ReturnInfo::DEPEND_ON = 3200;

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




