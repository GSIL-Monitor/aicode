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
    static const int DEVICE_ADDED_BY_CURRENT_USER_TYPE; //当前用户已添加此设备并且类型不同
    static const int CLOUD_STORAGE_NOT_PAID_USER;        //用户未开通付费云存储服务
    static const int UNDELETED_DEVICE_EXISTED_USER;      //用户仍有未删除的设备
    static const int ACCOUNT_LOGIN_AT_OTHER_TERMINAL;    //账户在其他终端登录
    static const int SHARED_DEVICE_NOT_ADDED_BY_USER;    //设备尚未被用户添加
    static const int SHARED_USERNAME_IS_INVALID_USER;    //用户名有误，找不到此用户
    static const int LOGIN_USING_TEMP_PASSWORD_USER;     //用户使用临时密码登录，必须修改密码
    static const int SHARED_USERID_IS_INVALID_USER;      //用户ID有误，找不到此用户
    static const int SHARING_DEVICE_UP_TO_LIMIT;             //用户主动分享设备达到上限

    //设备相关的错误代码
    static const int DEVICEID_NOT_EXISTED_DEV;           //设备ID不存在
    static const int PASSWORD_INVALID_DEV;               //设备密码不正确
    static const int DEVICE_DOMAIN_USED_DEV;             //设备二级域名已被使用
    static const int DEVICE_P2PID_USED_DEV;              //设备P2PID已被使用
    static const int DEVICE_DOMAINNAME_INVALID;          //设备二级域名不可用
    static const int DEVICE_NOT_ADDED_BY_USER;           //设备尚未被用户添加
    static const int DEVICE_LONGIN_REFUSED;           //设备登陆被拒绝

    //其他错误代码
    static const int SESSION_TIMEOUT;                    //会话超时
    static const int NO_ACCESS_PERMISSION;               //没有访问权限
    static const int INPUT_PARAMETER_TOO_MUCH;           //输入参数过多
    static const int INPUT_PARAMETER_TOO_LESS;           //输入参数不足

    static const int SENSOR_TYPE_DUPLICATE;        //传感器类型重复

    static const int CMS_P2PID_DUPLICATE;            //CMS注册的p2pid与现有的重复

    static const int DEPEND_ON;                             //存在依赖关系

    static const int ENTRANCE_DEVICE_ALREADY_BINDED; //出入口对应的设备已经被绑定过了，不能重复绑定

    static const int ENTRANCE_NAME_DUPLICATE;      //出入口名称重复

    static const int PRODUCT_ALREADY_USED; //产品已经使用

    static int RetCode();

    static void RetCode(const int iRetCode);

private:
    static boost::thread_specific_ptr<int> ms_iRetCode;

};


#endif
