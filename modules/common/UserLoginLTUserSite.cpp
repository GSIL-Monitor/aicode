#include <iostream>
#include <algorithm>

#include "json/json.h"

#include "CommonUtility.h"
#include "HttpClient.h"
#include "RC4Encrypt.h"
#include "LogRLD.h"

#include "UserLoginLTUserSite.h"

using std::string;

UserLoginLTUserSite::UserLoginLTUserSite(const string &url, const string &rc4Key) : LTUserSiteUrl(url), LTUserSiteRC4Key(rc4Key)
{

}


UserLoginLTUserSite::~UserLoginLTUserSite()
{

}

int UserLoginLTUserSite::Login(const string &userName, const string &password)
{
    int ret = LOGIN_FAILED;
    RC4Encrypt rc4;
    string postData;

    //{"ue":"�û�email","up":"�û�����","token":"�ֻ�token","ptype":"�ֻ�����","plang":"�ֻ�����","flag":"��¼����"} 
    //��1= ������¼  2=��ȡ��Ϣ 3=���µ�¼��
    string upperUserName = userName;
    transform(userName.begin(), userName.end(), upperUserName.begin(), ::toupper);
    postData.append("{\"ue\":\"");
    postData.append(rc4.RC4EncryptBase64(upperUserName, LTUserSiteRC4Key));

    postData.append("\",\"up\":\"");
    postData.append(rc4.RC4EncryptBase64(password, LTUserSiteRC4Key));

    postData.append("\",\"token\":\"");
    postData.append(rc4.RC4EncryptBase64("CZYTEST", LTUserSiteRC4Key));

    postData.append("\",\"ptype\":\"");
    postData.append(rc4.RC4EncryptBase64("3", LTUserSiteRC4Key));

    postData.append("\",\"plang\":\"");
    postData.append(rc4.RC4EncryptBase64("1", LTUserSiteRC4Key));

    postData.append("\",\"flag\":\"");
    postData.append(rc4.RC4EncryptBase64("3", LTUserSiteRC4Key));

    postData.append("\"}");

    LOG_INFO_RLD("Post data: " << postData);

    string respData;
    HttpClient httpClient;
    if (CURLE_OK != httpClient.Post(LTUserSiteUrl, postData, respData))
    {
        LOG_ERROR_RLD("Http post failed, url is: " << LTUserSiteUrl << " and postData is: " << postData);
        return ret;
    }

    LOG_INFO_RLD("Response data: " << respData);

    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(respData, value))
    {
        LOG_ERROR_RLD("Output parameter incorrect, illegal json farmat: " << respData);
        return ret;
    }

    //����json�еĶ���
    string res = value["re"].asString();
    res = rc4.RC4DecryptBase64(res, LTUserSiteRC4Key);

    //{"re":"��½���","dlist":"�豸�б�","clist":"�ղ��б�"} 
    //-1=ͨ���쳣 1=��½�ɹ� 2=��½ʧ�ܣ��û�email��������� 3=�������� 4=�û���ͬһ���ֻ��˵�½ 5=��½�ɹ���token��һ������Ҫ�ͻ��˶������豸���´򿪺͹رձ���

    int iRes = atoi(res.c_str());
    switch (iRes)
    {
    case -1:
        ret = LOGIN_FAILED;
        LOG_ERROR_RLD("Communication failure");
        break;
    case 1:
        ret = LOGIN_OK;
        LOG_INFO_RLD("Login seccessful");
        break;
    case 2:
        ret = INVALID_USERNAME_OR_PASSWORD;
        LOG_ERROR_RLD("Usernam or password is incorrect");
        break;
    case 3:
        ret = LOGIN_FAILED;
        LOG_ERROR_RLD("Input parameter incorrect");
        break;
    case 4:
        ret = LOGIN_FAILED;
        LOG_ERROR_RLD("Login failed");
        break;
    case 5:
        ret = LOGIN_OK;
        LOG_INFO_RLD("Login seccessful, communication session is different");
        break;
    default:
        ret = LOGIN_FAILED;
        LOG_ERROR_RLD("Login failed, unkown login return code");
        break;
    }

    if (LOGIN_OK != ret)
    {
        return ret;
    }

    //string dlist = value["dlist"].asString();
    //if (!dlist.empty())
    //{
    //    dlist = rc4.RC4DecryptBase64(dlist, LTUserSiteRC4Key);
    //    LOG_INFO_RLD("Device list: " << dlist);
    //}

    //string clist = value["clist"].asString();
    //if (!clist.empty())
    //{
    //    clist = rc4.RC4DecryptBase64(clist, LTUserSiteRC4Key);
    //    LOG_INFO_RLD("Collect list: " << clist);
    //}

    return ret;
}
