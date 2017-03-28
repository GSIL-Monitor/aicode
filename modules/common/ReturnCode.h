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

    //设备相关的错误代码
    static const int DEVICEID_NOT_EXISTED_DEV;           //设备ID不存在
    static const int PASSWORD_INVALID_DEV;               //设备密码不正确

    //其他错误代码
    static const int INPUT_PARAMETER_TOO_LESS;           //输入参数不足

    static int RetCode();

    static void RetCode(const int iRetCode);

private:
    static boost::thread_specific_ptr<int> ms_iRetCode;

};


#endif
