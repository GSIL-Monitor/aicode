#include <boost/scope_exit.hpp>
#include "Mp4FileHandler.h"
#include "InterProcessHandler.h"
#include "LogRLD.h"
#include "json/json.h"
#include <arpa/inet.h>

Mp4FileHandler::Mp4FileHandler(const unsigned int uiRunTdNum) :
    m_MsgReceiver(new InterProcessHandler(InterProcessHandler::RECEIVE_MODE, "mp4_req", uiRunTdNum)),
    m_MsgSender(new InterProcessHandler(InterProcessHandler::SEND_MODE, "mp4_rsp")),
    m_Mp4RspRunner(1)
{
    m_MsgReceiver->SetMsgOfReceivedHandler(boost::bind(&Mp4FileHandler::Mp4MsgHandler, this, _1));

    m_cBuffer = new unsigned char[MAX_BUFFER_SIZE];
    memset(m_cBuffer, 0, MAX_BUFFER_SIZE);
}

Mp4FileHandler::~Mp4FileHandler()
{
    m_Mp4RspRunner.Stop();

    if (m_cBuffer != NULL)
    {
        delete m_cBuffer;
        m_cBuffer = NULL;
    }
}

void Mp4FileHandler::Run()
{
    m_Mp4RspRunner.Run();
    m_MsgReceiver->RunReceivedMsg(true);
}

void Mp4FileHandler::Mp4MsgHandler(const std::string &strMsg)
{
    LOG_INFO_RLD("Mp4 file msg is " << strMsg);

    bool blResult = false;

    std::string strVideoPath;
    std::string strAudioPath;

    Json::Reader reader;
    Json::Value root;
    Json::Value fileid;
    Json::Value eventid;
    Json::Value path;

    BOOST_SCOPE_EXIT(&blResult, &m_MsgSender, &fileid, &eventid, &strVideoPath, &strAudioPath)
    {
        std::string strRsp;
        Json::Value jsBody;
        if (blResult)
        {
            jsBody["retcode"] = "0";
            jsBody["fileid"] = fileid.asString() + ".mp4";
            jsBody["eventid"] = eventid.asString();
        }
        else
        {
            jsBody["retcode"] = "-1";
        }

        Json::FastWriter fastwriter;
        strRsp = fastwriter.write(jsBody);

        remove(strVideoPath.c_str());
        remove(strAudioPath.c_str());

        m_MsgSender->SendMsg(strRsp);
        //m_Mp4RspRunner.Post(boost::bind(&InterProcessHandler::SendMsg, m_MsgSender, strBody));
    }
    BOOST_SCOPE_EXIT_END

        if (!reader.parse(strMsg, root))
        {
            LOG_ERROR_RLD("Mp4MsgHandler failed, parse receive message error, raw message is : " << strMsg);
            return;
        }

    fileid = root["fileid"];
    if (fileid.isNull() || !fileid.isString() || fileid.asString().empty())
    {
        LOG_ERROR_RLD("Mp4MsgHandler failed, receive message fromat error, raw message is: " << strMsg);
        return;
    }

    eventid = root["eventid"];
    if (eventid.isNull() || !eventid.isString() || eventid.asString().empty())
    {
        LOG_ERROR_RLD("Mp4MsgHandler failed, receive message fromat error, raw message is: " << strMsg);
        return;
    }

    path = root["localpath"];
    if (path.isNull() || !path.isString() || path.asString().empty())
    {
        LOG_ERROR_RLD("Mp4MsgHandler failed, receive message fromat error, raw message is: " << strMsg);
        return;
    }

    if (!SeparateVideoAndAudioFile(path.asString(), strVideoPath, strAudioPath))
    {
        LOG_ERROR_RLD("Mp4MsgHandler failed, separate file error, file id is " << fileid <<
            " and event id is " << eventid << " and file path is " << path);
        return;
    }

    std::string strMp4Path;
    if (!m_mp4Converter.ConvertVideo(strVideoPath, strAudioPath, m_videoInfo.usVideoWidth,
        m_videoInfo.usVideoHeight, m_videoInfo.ucFrameRate))
    {
        LOG_ERROR_RLD("Mp4MsgHandler failed, convert mp4 file error, file id is " << fileid <<
            " and event id is " << eventid << " and file path is " << path);
        return;
    }

    blResult = true;
    LOG_INFO_RLD("Mp4MsgHandler successful, file id is " << fileid << " and event id is " << eventid <<
        " and file path is " << path);
}

bool Mp4FileHandler::SeparateVideoAndAudioFile(const std::string &strFilePath, std::string &strVideoPath,
    std::string &strAudioPath)
{
    FILE *mixedFd = NULL;
    FILE *videoFd = NULL;
    FILE *audioFd = NULL;

    BOOST_SCOPE_EXIT(&mixedFd, &videoFd, &audioFd)
    {
        if (mixedFd != NULL)
        {
            fclose(mixedFd);
            mixedFd = NULL;
        }

        if (videoFd != NULL)
        {
            fclose(videoFd);
            videoFd = NULL;
        }

        if (audioFd != NULL)
        {
            fclose(audioFd);
            audioFd = NULL;
        }
    }
    BOOST_SCOPE_EXIT_END

        if ((mixedFd = fopen(strFilePath.c_str(), "rb")) == NULL)
        {
            LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, can not open input mixed file, path is " << strFilePath);
            return false;
        }

    std::string strTmp(strFilePath);
    std::string::size_type pos = strTmp.find_last_of("/");
    std::string strFile = strTmp.substr(pos + 1);
    std::string strPath = strTmp.erase(pos + 1);

    strVideoPath = strPath + "video_" + strFile;
    if ((videoFd = fopen(strTmp.c_str(), "wb")) == NULL)
    {
        LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, can not open video file, path is " << strVideoPath);
        return false;
    }

    strAudioPath = strPath + "audio_" + strFile;
    if ((audioFd = fopen(strTmp.c_str(), "wb")) == NULL)
    {
        LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, can not open audio file, path is " << strAudioPath);
        return false;
    }

    unsigned char frameType;
    while (fread(&frameType, sizeof(frameType), 1, mixedFd) == 1)
    {
        if (frameType == VIDEO_IFRAME || frameType == VIDEO_PFRAME)
        {
            memset(&m_videoInfo, 0, sizeof(VideoInfo));

            if (fread(&m_videoInfo, sizeof(VideoInfo), 1, mixedFd) != 1)
            {
                LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, read frame info error");
                return false;
            }

            unsigned int len = ntohl(m_videoInfo.uiPacketSize);
            if (fread(m_cBuffer, sizeof(char), len, mixedFd) != len)
            {
                LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, read frame data error");
                return false;
            }

            if (fwrite(m_cBuffer, sizeof(char), len, videoFd) != len)
            {
                LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, write frame data error");
                return false;
            }
        }
        else if (frameType == AUDIO_FRAME)
        {
            memset(&m_audioInfo, 0, sizeof(AudioInfo));

            if (fread(&m_audioInfo, sizeof(AudioInfo), 1, mixedFd) != 1)
            {
                LOG_ERROR_RLD("SeparateAudioAndAudioFile failed, read frame info error");
                return false;
            }

            unsigned int len = ntohl(m_audioInfo.uiPacketSize);
            if (fread(m_cBuffer, sizeof(char), len, mixedFd) != len)
            {
                LOG_ERROR_RLD("SeparateAudioAndAudioFile failed, read frame data error");
                return false;
            }

            if (fwrite(m_cBuffer, sizeof(char), len, audioFd) != len)
            {
                LOG_ERROR_RLD("SeparateAudioAndAudioFile failed, write frame data error");
                return false;
            }
        }
        else
        {
            LOG_ERROR_RLD("SeparateAudioAndAudioFile failed, unknown frame type: " << frameType);
            return false;
        }
    }

    return true;
}
