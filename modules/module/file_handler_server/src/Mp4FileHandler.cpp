#include "Mp4FileHandler.h"
#include "InterProcessHandler.h"
#include "LogRLD.h"

Mp4FileHandler::Mp4FileHandler(const unsigned int uiRunTdNum) : 
m_MsgSender(new InterProcessHandler(InterProcessHandler::RECEIVE_MODE, "mp4file", uiRunTdNum))
{
    m_MsgSender->SetMsgOfReceivedHandler(boost::bind(&Mp4FileHandler::Mp4MsgHandler, this, _1));
}


Mp4FileHandler::~Mp4FileHandler()
{
}

void Mp4FileHandler::Run()
{
    m_MsgSender->RunReceivedMsg();
}

void Mp4FileHandler::Mp4MsgHandler(const std::string &strMsg)
{
    LOG_INFO_RLD("Mp4 file msg is " << strMsg);

}
