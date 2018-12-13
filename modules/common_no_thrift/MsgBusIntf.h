#pragma once
#ifndef _MSG_BUS_INTF_
#define _MSG_BUS_INTF_

#include <string>

/************************************************************************/
/* ��Ϣ���߽ӿ��࣬�����ṩ������Ϣ����ʵ�ֵ���Ϣ�ַ����ơ�        
 * Author������
 * Date��2016-11-18*/
/************************************************************************/
class MsgBusIntf
{
public:

    /*
     * ���캯��
     **/
    MsgBusIntf(){};
    
    /*
     * ����������
     **/
    virtual ~MsgBusIntf(){};

    /*
    * ��ʼ������
    * @return true���ɹ���false��ʧ��
    **/
    virtual bool Init() = 0;

    /*
     * �㲥��Ϣ
     * @param strKey ������Ϣ��Key������ʶ����㲥������Ϣ������
     * @param strMsg ���㲥������Ϣ����
     **/
    virtual void BroadcastMsg(const std::string &strKey, const std::string &strMsg) = 0;


    /*
     * ������Ϣ
     * @param strKey ������Ϣ��Key������ʶ���������Ϣ������
     * @param strMsg �������յ�����Ϣ����
     **/
    virtual void ListenMsg(const std::string &strKey, std::string &strMsg) = 0;

    
};


#endif