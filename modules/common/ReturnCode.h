#ifndef _RETURN_CODE_
#define _RETURN_CODE_

#include <boost/thread/tss.hpp>
#include "boost/noncopyable.hpp"
#include <string>
class ReturnInfo : public boost::noncopyable
{
public:
    ReturnInfo();
    ~ReturnInfo();

    static const int SUCCESS_CODE;
    static const std::string SUCCESS_INFO;

    static const int FAILED_CODE;
    static const std::string FAILED_INFO;

    //用户相关的错误代码
    static const int USERNAME_EXISTED_USER;              //用户名已存在
    static const int USERNAME_NOT_EXISTED_USER;          //用户名不存在
    static const int USERID_NOT_EXISTED_USER;            //用户ID不存在
    static const int PASSWORD_INVALID_USER;              //用户密码不正确
    static const int USERNAME_OR_PASSWORD_INVALID_USER;  //用户名或者密码不正确
    static const int EMAIL_NOT_MATCHED_USER;             //邮箱与用户名不匹配
    static const int DEVICE_IS_ADDED_USER;               //设备已经被添加
    static const int DEVICE_NOT_BELONG_TO_USER;          //设备不归属于用户
    static const int DEVICE_NOT_RECORDED_USER;           //设备未记录
    static const int DEVICE_P2PID_NOT_RECORDED_USER;     //设备P2PID未记录
    static const int DEVICE_ADDED_BY_CURRENT_USER;       //当前用户已添加此设备
    static const int CLOUD_STORAGE_NOT_PAID_USER;        //用户未开通付费云存储服务
    static const int UNDELETED_DEVICE_EXISTED_USER;      //用户仍有未删除的设备
    static const int ACCOUNT_LOGIN_AT_OTHER_TERMINAL;    //账户在其他终端登录

    //设备相关的错误代码
    static const int DEVICEID_NOT_EXISTED_DEV;           //设备ID不存在
    static const int PASSWORD_INVALID_DEV;               //设备密码不正确
    static const int DEVICE_DOMAIN_USED_DEV;             //设备二级域名已被使用
    static const int DEVICE_P2PID_USED_DEV;              //设备P2PID已被使用
    static const int DEVICE_DOMAINNAME_INVALID;          //设备二级域名不可用
    static const int DEVICE_NOT_ADDED_BY_USER;           //设备尚未被用户添加

    //其他错误代码
    static const int INPUT_PARAMETER_TOO_MUCH;           //输入参数过多
    static const int INPUT_PARAMETER_TOO_LESS;           //输入参数不足

    static int RetCode();

    static void RetCode(const int iRetCode);

private:
    static boost::thread_specific_ptr<int> ms_iRetCode;

};


#endif
