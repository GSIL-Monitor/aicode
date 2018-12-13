#ifndef _MSG_BUS_CLIENT_
#define _MSG_BUS_CLIENT_

#include "MsgBusIntf.h"
#include "boost/shared_ptr.hpp"
#include <string>

/************************************************************************/
/* ��Ϣ���߿ͻ��ˣ�ʹ�÷�������ģʽ�����ͺͽ�����Ϣ��                 
 * Author������
 * Date��2016-11-18*/
/************************************************************************/

namespace AmqpClient
{
    class Channel;
}

class MsgBusClientFanout : public MsgBusIntf
{
public:
    
    /*
     * ���캯��
     **/
    MsgBusClientFanout(const std::string &strExchangeName, const std::string &strHost = "127.0.0.1", int iPort = 5672,
        const std::string &strUsername = "guest",
        const std::string &strPassword = "guest" );

    /*
     * ��������
     **/
    ~MsgBusClientFanout();

    /*
    * ��ʼ���������÷��������ڶ�����֮���ڵ�����������֮ǰ���е��á�
    * ȷ�������Ѿ��ɹ���ʼ����ɡ�
    * @return true���ɹ���false��ʧ��
    **/
    virtual bool Init();

    /*
    * �㲥��Ϣ
    * @param strKey ������Ϣ��Key������ʶ����㲥������Ϣ������
    * @param strMsg ���㲥������Ϣ����
    **/
    virtual void BroadcastMsg(const std::string &strKey, const std::string &strMsg);


    /*
    * ������Ϣ
    * @param strKey ������Ϣ��Key������ʶ���������Ϣ������
    * @param strMsg �������յ�����Ϣ����
    **/
    virtual void ListenMsg(const std::string &strKey, std::string &strMsg);

private:
    


private:
    const std::string m_strExchangeName;
    
    std::string m_strHost;
    int m_iPort;
    std::string m_strUsername;
    std::string m_strPassword;
       
    boost::shared_ptr<AmqpClient::Channel> m_pChannel;
    std::string m_strQueueName;
    std::string m_strConsumerTag;

};


#endif