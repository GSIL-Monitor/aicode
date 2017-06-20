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

    //�û���صĴ������
    static const int USERNAME_EXISTED_USER;              //�û����Ѵ���
    static const int USERNAME_NOT_EXISTED_USER;          //�û���������
    static const int USERID_NOT_EXISTED_USER;            //�û�ID������
    static const int PASSWORD_INVALID_USER;              //�û����벻��ȷ
    static const int USERNAME_OR_PASSWORD_INVALID_USER;  //�û����������벻��ȷ
    static const int EMAIL_NOT_MATCHED_USER;             //�������û�����ƥ��
    static const int DEVICE_IS_ADDED_USER;               //�豸�Ѿ������
    static const int DEVICE_NOT_BELONG_TO_USER;          //�豸���������û�
    static const int DEVICE_NOT_RECORDED_USER;           //�豸δ��¼
    static const int DEVICE_P2PID_NOT_RECORDED_USER;     //�豸P2PIDδ��¼
    static const int DEVICE_ADDED_BY_CURRENT_USER;       //��ǰ�û�����Ӵ��豸
    static const int CLOUD_STORAGE_NOT_PAID_USER;        //�û�δ��ͨ�����ƴ洢����
    static const int UNDELETED_DEVICE_EXISTED_USER;      //�û�����δɾ�����豸
    static const int ACCOUNT_LOGIN_AT_OTHER_TERMINAL;    //�˻��������ն˵�¼

    //�豸��صĴ������
    static const int DEVICEID_NOT_EXISTED_DEV;           //�豸ID������
    static const int PASSWORD_INVALID_DEV;               //�豸���벻��ȷ
    static const int DEVICE_DOMAIN_USED_DEV;             //�豸���������ѱ�ʹ��
    static const int DEVICE_P2PID_USED_DEV;              //�豸P2PID�ѱ�ʹ��
    static const int DEVICE_DOMAINNAME_INVALID;          //�豸��������������
    static const int DEVICE_NOT_ADDED_BY_USER;           //�豸��δ���û����

    //�����������
    static const int INPUT_PARAMETER_TOO_MUCH;           //�����������
    static const int INPUT_PARAMETER_TOO_LESS;           //�����������

    static int RetCode();

    static void RetCode(const int iRetCode);

private:
    static boost::thread_specific_ptr<int> ms_iRetCode;

};


#endif
