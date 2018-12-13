#pragma once
#ifndef _MSG_BUS_INTF_
#define _MSG_BUS_INTF_

#include <string>

/************************************************************************/
/* 消息总线接口类，用来提供基于消息队列实现的消息分发机制。        
 * Author：尹宾
 * Date：2016-11-18*/
/************************************************************************/
class MsgBusIntf
{
public:

    /*
     * 构造函数
     **/
    MsgBusIntf(){};
    
    /*
     * 虚析构函数
     **/
    virtual ~MsgBusIntf(){};

    /*
    * 初始化方法
    * @return true，成功；false，失败
    **/
    virtual bool Init() = 0;

    /*
     * 广播消息
     * @param strKey 发布消息的Key，用来识别待广播发布消息的类型
     * @param strMsg 待广播发布消息内容
     **/
    virtual void BroadcastMsg(const std::string &strKey, const std::string &strMsg) = 0;


    /*
     * 监听消息
     * @param strKey 监听消息的Key，用来识别待监听消息的类型
     * @param strMsg 监听接收到的消息内容
     **/
    virtual void ListenMsg(const std::string &strKey, std::string &strMsg) = 0;

    
};


#endif