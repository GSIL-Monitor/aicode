#ifndef __MP4_FILE_HANDLER__
#define __MP4_FILE_HANDLER__

#include "NetComm.h"
#include "MP4Encoder.h"
#include "MP4Converter.h"

class InterProcessHandler;

class Mp4FileHandler
{
public:
    static const unsigned char VIDEO_IFRAME = 0;
    static const unsigned char VIDEO_PFRAME = 1;
    static const unsigned char AUDIO_FRAME = 2;

    static const int MAX_BUFFER_SIZE = 500 * 1024;

    Mp4FileHandler(const unsigned int uiRunTdNum);
    ~Mp4FileHandler();

    void Run();

    struct VideoInfo
    {
        //unsigned char ucFrameType;
        unsigned char ucVideoCode;
        unsigned char ucFrameRate;
        //char reserved1;
        unsigned short usVideoWidth;
        unsigned short usVideoHeight;
        unsigned int uiPacketSize;
        //char reserved2[4];
    };

    struct AudioInfo
    {
        //unsigned char ucFrameType;
        unsigned char ucAudioCode;
        unsigned char ucSampleRate;
        unsigned char ucBitRate;
        unsigned int uiPacketSize;
        //char reserved[4];
    };

private:
    void Mp4MsgHandler(const std::string &strMsg);

    bool SeparateVideoAndAudioFile(const std::string &strFilePath, std::string &strVideoPath, std::string &strAudioPath);

private:
    boost::shared_ptr<InterProcessHandler> m_MsgReceiver;
    boost::shared_ptr<InterProcessHandler> m_MsgSender;

    unsigned char *m_cBuffer;

    VideoInfo m_videoInfo;
    AudioInfo m_audioInfo;

    MP4Encoder m_mp4Encoder;
    MP4Converter m_mp4Converter;
    Runner m_Mp4RspRunner;
};

#endif
