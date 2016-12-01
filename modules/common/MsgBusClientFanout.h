#ifndef _MSG_BUS_CLIENT_
#define _MSG_BUS_CLIENT_

#include "MsgBusIntf.h"
#include "boost/shared_ptr.hpp"
#include <string>

/************************************************************************/
/* 消息总线客户端，使用发布订阅模式来发送和接收消息。                 
 * Author：尹宾
 * Date：2016-11-18*/
/************************************************************************/

namespace AmqpClient
{
    class Channel;
}

class MsgBusClientFanout : public MsgBusIntf
{
public:
    
    /*
     * 构造函数
     **/
    MsgBusClientFanout(const std::string &strExchangeName, const std::string &strHost = "127.0.0.1", int iPort = 5672,
        const std::string &strUsername = "guest",
        const std::string &strPassword = "guest" );

    /*
     * 析构函数
     **/
    ~MsgBusClientFanout();

    /*
    * 初始化方法，该方法必须在对象构造之后，在调用其它方法之前先行调用。
    * 确保对象已经成功初始化完成。
    * @return true，成功；false，失败
    **/
    virtual bool Init();

    /*
    * 广播消息
    * @param strKey 发布消息的Key，用来识别待广播发布消息的类型
    * @param strMsg 待广播发布消息内容
    **/
    virtual void BroadcastMsg(const std::string &strKey, const std::string &strMsg);


    /*
    * 监听消息
    * @param strKey 监听消息的Key，用来识别待监听消息的类型
    * @param strMsg 监听接收到的消息内容
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