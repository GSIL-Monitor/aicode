#include "ReturnCode.h"

const int ReturnInfo::SUCCESS_CODE = 0;

const std::string ReturnInfo::SUCCESS_INFO = "ok";

const int ReturnInfo::FAILED_CODE = 1;

const std::string ReturnInfo::FAILED_INFO = "failed";

const int ReturnInfo::USERNAME_EXISTED_USER = 1000;                //�û����Ѵ���
const int ReturnInfo::USERNAME_NOT_EXISTED_USER = 1001;            //�û���������
const int ReturnInfo::USERID_NOT_EXISTED_USER = 1002;              //�û�ID������
const int ReturnInfo::PASSWORD_INVALID_USER = 1003;                //�û����벻��ȷ
const int ReturnInfo::USERNAME_OR_PASSWORD_INVALID_USER = 1004;    //�û����������벻��ȷ
const int ReturnInfo::EMAIL_NOT_MATCHED_USER = 1005;               //�������û�����ƥ��
const int ReturnInfo::DEVICE_IS_ADDED_USER = 1006;                 //�豸�Ѿ������
const int ReturnInfo::DEVICE_NOT_BELONG_TO_USER = 1007;            //�豸���������û�
const int ReturnInfo::DEVICE_NOT_RECORDED_USER = 1008;             //�豸δ��¼
const int ReturnInfo::DEVICE_P2PID_NOT_RECORDED_USER = 1009;       //�豸P2PIDδ��¼
const int ReturnInfo::DEVICE_ADDED_BY_CURRENT_USER = 1010;         //��ǰ�û�����Ӵ��豸
const int ReturnInfo::CLOUD_STORAGE_NOT_PAID_USER = 1011;          //�û�δ��ͨ�����ƴ洢����
const int ReturnInfo::UNDELETED_DEVICE_EXISTED_USER = 1012;        //�û�����δɾ�����豸
const int ReturnInfo::ACCOUNT_LOGIN_AT_OTHER_TERMINAL = 1013;      //�˻��������ն˵�¼
const int ReturnInfo::SHARED_DEVICE_NOT_ADDED_BY_USER = 1014;      //�豸��δ���û����
const int ReturnInfo::SHARED_USERNAME_IS_INVALID_USER = 1015;      //�û��������Ҳ������û�
const int ReturnInfo::LOGIN_USING_TEMP_PASSWORD_USER = 1016;       //�û�ʹ����ʱ�����¼�������޸�����
const int ReturnInfo::SHARED_USERID_IS_INVALID_USER = 1017;        //�û�ID�����Ҳ������û�

const int ReturnInfo::SHARING_DEVICE_UP_TO_LIMIT = 3001;

const int ReturnInfo::DEVICE_DOMAIN_USED_DEV = 2000;               //�豸���������ѱ�ʹ��
const int ReturnInfo::DEVICE_P2PID_USED_DEV = 2001;                //�豸P2PID�ѱ�ʹ��
const int ReturnInfo::DEVICE_DOMAINNAME_INVALID = 2002;            //�豸��������������
const int ReturnInfo::DEVICE_NOT_ADDED_BY_USER = 2003;             //�豸��δ���û����
//δʹ�ã�ռλ
const int ReturnInfo::DEVICEID_NOT_EXISTED_DEV = 2998;             //�豸ID������
const int ReturnInfo::PASSWORD_INVALID_DEV = 2999;                 //�豸���벻��ȷ

const int ReturnInfo::SESSION_TIMEOUT = 996;                       //�Ự��ʱ
const int ReturnInfo::NO_ACCESS_PERMISSION = 997;                  //û�з���Ȩ��
const int ReturnInfo::INPUT_PARAMETER_TOO_MUCH = 998;              //�����������
const int ReturnInfo::INPUT_PARAMETER_TOO_LESS = 999;              //�����������

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




