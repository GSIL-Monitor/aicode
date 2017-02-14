#ifndef _USER_LOGIN_LTUSERSITE_
#define _USER_LOGIN_LTUSERSITE_

#include <string>

//登录结果，1正常登录，2用户或密码不正确，3用户不存在，4用户已被锁定, 5登录失败
#define LOGIN_OK                      1
#define INVALID_USERNAME_OR_PASSWORD  2
#define USERNAME_INVALID              3
#define USER_LOCKED                   4
#define LOGIN_FAILED                  5

/*
用户登录，在本地数据库查询不到用户登录信息时，继续发送登录请求到浪涛服务器尝试登录。
通过HTTP方式请求数据
*/
class UserLoginLTUserSite
{
public:
	UserLoginLTUserSite(const std::string &url, const std::string &rc4Key);
	~UserLoginLTUserSite();

	int Login(const std::string &userName, const std::string &password);

private:
	std::string LTUserSiteUrl;
	std::string LTUserSiteRC4Key;

};

#endif