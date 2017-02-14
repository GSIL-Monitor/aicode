#ifndef _USER_LOGIN_LTUSERSITE_
#define _USER_LOGIN_LTUSERSITE_

#include <string>

//��¼�����1������¼��2�û������벻��ȷ��3�û������ڣ�4�û��ѱ�����, 5��¼ʧ��
#define LOGIN_OK                      1
#define INVALID_USERNAME_OR_PASSWORD  2
#define USERNAME_INVALID              3
#define USER_LOCKED                   4
#define LOGIN_FAILED                  5

/*
�û���¼���ڱ������ݿ��ѯ�����û���¼��Ϣʱ���������͵�¼�������η��������Ե�¼��
ͨ��HTTP��ʽ��������
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