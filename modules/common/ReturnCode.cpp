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

const int ReturnInfo::DEVICE_DOMAIN_USED_DEV = 2000;               //�豸���������ѱ�ʹ��
const int ReturnInfo::DEVICE_P2PID_USED_DEV = 2001;                //�豸P2PID�ѱ�ʹ��
//δʹ�ã�ռλ
const int ReturnInfo::DEVICEID_NOT_EXISTED_DEV = 2998;             //�豸ID������
const int ReturnInfo::PASSWORD_INVALID_DEV = 2999;                 //�豸���벻��ȷ

const int ReturnInfo::INPUT_PARAMETER_TOO_MUCH = 998;              //�����������
const int ReturnInfo::INPUT_PARAMETER_TOO_LESS = 999;              //�����������

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




