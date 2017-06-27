#include "Mp4FileHandler.h"
#include "InterProcessHandler.h"
#include "LogRLD.h"

Mp4FileHandler::Mp4FileHandler(const unsigned int uiRunTdNum) : 
m_MsgReceiver(new InterProcessHandler(InterProcessHandler::RECEIVE_MODE, "mp4_req", uiRunTdNum)),
m_MsgSender(new InterProcessHandler(InterProcessHandler::SEND_MODE, "mp4_rsp")),
m_Mp4RspRunner(1)
{
    m_MsgReceiver->SetMsgOfReceivedHandler(boost::bind(&Mp4FileHandler::Mp4MsgHandler, this, _1));
}


Mp4FileHandler::~Mp4FileHandler()
{
    m_Mp4RspRunner.Stop();
}

void Mp4FileHandler::Run()
{
    m_Mp4RspRunner.Run();
    m_MsgReceiver->RunReceivedMsg(true);    
}

void Mp4FileHandler::Mp4MsgHandler(const std::string &strMsg)
{
    LOG_INFO_RLD("Mp4 file msg is " << strMsg);

    //mp4文件处理开始





    //mp4文件处理结束
    
    m_Mp4RspRunner.Post(boost::bind(&InterProcessHandler::SendMsg, m_MsgSender, strMsg));    
}
