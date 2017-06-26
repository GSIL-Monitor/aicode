#ifndef __MP4_FILE_HANDLER__
#define __MP4_FILE_HANDLER__

#include "NetComm.h"
#include "MP4Encoder.h"

class InterProcessHandler;

class Mp4FileHandler
{
public:
    Mp4FileHandler(const unsigned int uiRunTdNum);
    ~Mp4FileHandler();

    void Run();

private:
    void Mp4MsgHandler(const std::string &strMsg);

private:
    boost::shared_ptr<InterProcessHandler> m_MsgSender;

    MP4Encoder m_Mp4Encoder;
};

#endif
