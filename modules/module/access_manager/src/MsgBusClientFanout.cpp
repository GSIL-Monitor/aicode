#include "MsgBusClientFanout.h"

#include "SimpleAmqpClient/SimpleAmqpClient.h"
#include "LogRLD.h"
#include "CommonUtility.h"

using namespace AmqpClient;


MsgBusClientFanout::MsgBusClientFanout(const std::string &strExchangeName, const std::string &strHost /*= "127.0.0.1"*/, int iPort /*= 5672*/,
    const std::string &strUsername /*= "guest"*/, const std::string &strPassword /*= "guest" */) : MsgBusIntf(), 
    m_strExchangeName(strExchangeName), m_strHost(strHost), m_iPort(iPort), m_strUsername(strUsername), m_strPassword(strPassword)
{
    LOG_INFO_RLD("MsgBusClientFanout constructed, host is " << m_strHost << " port is " << m_iPort << 
        " user name is " << m_strUsername << " pwd is " << m_strPassword << " and exange name is " << m_strExchangeName);
}

MsgBusClientFanout::~MsgBusClientFanout()
{
    //m_pChannel->DeleteExchange(m_strExchangeName, false);
}

bool MsgBusClientFanout::Init()
{
    try
    {
        Channel::ptr_t channel = Channel::Create(m_strHost, m_iPort, m_strUsername, m_strPassword);

        channel->DeclareExchange(m_strExchangeName, Channel::EXCHANGE_TYPE_FANOUT);
        
        m_pChannel = channel;

    }
    catch (AmqpException &e)
    {
        LOG_ERROR_RLD("MsgBusClientFanout init failed, error :" << e.what());
        return false;
    }

    LOG_INFO_RLD("MsgBusClientFanout init success and exchange name is " << m_strExchangeName);

    return true;
}

void MsgBusClientFanout::BroadcastMsg(const std::string &strKey, const std::string &strMsg)
{
    try
    {
        m_pChannel->BasicPublish(m_strExchangeName, "", BasicMessage::Create(strMsg));
        
    }
    catch (MessageReturnedException &e)
    {
        LOG_ERROR_RLD("Broadcast msg failed and error msg is " << e.what() << " error msg body is " << e.message()->Body());
        return;
    }

    LOG_INFO_RLD("Broadcast msg success.");
}

void MsgBusClientFanout::ListenMsg(const std::string &strKey, std::string &strMsg)
{
    try
    {
        if (m_strQueueName.empty())
        {
            //const std::string &strRadom = CreateUUID();
            //LOG_INFO_RLD("Radom string is " << strRadom);
            std::string queue = m_pChannel->DeclareQueue("");
            LOG_INFO_RLD("Declare queue is ========" << queue);
            m_pChannel->BindQueue(queue, m_strExchangeName, "");
            LOG_INFO_RLD("=============================");
            m_strQueueName = queue;
            m_strConsumerTag = CreateUUID();

            LOG_INFO_RLD("Queue name is " << m_strQueueName << " and consumer tag is " << m_strConsumerTag);

            const std::string &strTagReturned = m_pChannel->BasicConsume(m_strQueueName, m_strConsumerTag);
            LOG_INFO_RLD("Consumer tag that returned is " << strTagReturned);
        }
        
        

        //Envelope::ptr_t env;

        //if (m_pChannel->BasicConsumeMessage(m_strConsumerTag, env))
        const std::string &strReceiveMsg = m_pChannel->BasicConsumeMessage()->Message()->Body();
        LOG_INFO_RLD("Receive msg is " << strReceiveMsg);

        ////
        //{
        //    strMsg = env->Message()->Body();

        //    LOG_INFO_RLD("Envelope received: \n"
        //        << " Exchange: " << env->Exchange()
        //        << "\n Routing key: " << env->RoutingKey()
        //        << "\n Consumer tag: " << env->ConsumerTag()
        //        << "\n Delivery tag: " << env->DeliveryTag()
        //        << "\n Redelivered: " << env->Redelivered()
        //        << "\n Body: " << env->Message()->Body());
        //}
        //else
        //{
        //    LOG_ERROR_RLD("BasicConsumeMessage failed.");
        //    return;
        //}
       

    }
    catch (AmqpException &e)
    {
        LOG_ERROR_RLD("MsgBusClientFanout init failed, error :" << e.what());
        return;
    }

    LOG_INFO_RLD("MsgBusClientFanout init success and exchange name is " << m_strExchangeName << " queue name is " << m_strQueueName);

    

}

