#include <boost/scope_exit.hpp>
#include "Mp4FileHandler.h"
#include "InterProcessHandler.h"
#include "LogRLD.h"
#include "json/json.h"
#include <arpa/inet.h>

Mp4FileHandler::Mp4FileHandler()
{
    m_cBuffer = new unsigned char[MAX_BUFFER_SIZE];
    memset(m_cBuffer, 0, MAX_BUFFER_SIZE);
}

Mp4FileHandler::~Mp4FileHandler()
{
    if (m_cBuffer != NULL)
    {
        delete m_cBuffer;
        m_cBuffer = NULL;
    }
}

void Mp4FileHandler::Mp4MsgHandler(const std::string &strMsg, std::string &strResult)
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
    std::string thumbnail;

    BOOST_SCOPE_EXIT(&blResult, &fileid, &eventid, &thumbnail, &strVideoPath, &strAudioPath, &strResult)
    {
        std::string strRsp;
        Json::Value jsBody;
        if (blResult)
        {
            jsBody["retcode"] = "0";
            jsBody["fileid"] = fileid.asString() + ".mp4";
            jsBody["eventid"] = eventid.asString();
            jsBody["thumbnail_fileid"] = thumbnail;
        }
        else
        {
            jsBody["retcode"] = "-1";
        }

        Json::FastWriter fastwriter;
        strRsp = fastwriter.write(jsBody);

        remove(strVideoPath.c_str());
        remove(strAudioPath.c_str());

        strResult = strRsp;
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

    Json::Value imgsize = root["img_resolution"];
    if (imgsize.isNull() || !imgsize.isString() || imgsize.asString().empty())
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

    //std::string strMp4Path;
    //if (!m_mp4Converter.ConvertVideo(strVideoPath, strAudioPath, ntohs(m_videoInfo.usVideoWidth),
    //    ntohs(m_videoInfo.usVideoHeight), m_videoInfo.ucFrameRate))
    //{
    //    LOG_ERROR_RLD("Mp4MsgHandler failed, convert mp4 file error, file id is " << fileid <<
    //        " and event id is " << eventid << " and file path is " << path);
    //    return;
    //}

    char cmd[1024] = { 0 };
    int size = sizeof(cmd);
    const char *fmt = "./mp4box.sh %s %s %u %s.mp4";
    snprintf(cmd, size, fmt, strVideoPath.c_str(), strAudioPath.c_str(), m_videoInfo.ucFrameRate, path.asCString());

    LOG_INFO_RLD("Mp4MsgHandler exec mp4box cmd: " << cmd);
    int status = system(cmd);
    if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status))
    {
        LOG_ERROR_RLD("Mp4MsgHandler failed, convert mp4 file error, file id is " << fileid <<
            " and event id is " << eventid << " and file path is " << path);
        return;
    }

    //生成视频缩略图
    memset(cmd, 0, size);
    snprintf(cmd, size, "./thumbnail.sh %s %s", path.asCString(), imgsize.asCString());

    LOG_INFO_RLD("Mp4MsgHandler exec ffmpeg cmd: " << cmd);
    status = system(cmd);
    if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status))
    {
        LOG_ERROR_RLD("Mp4MsgHandler failed, generate video thumbnails error, file id is " << fileid <<
            " and event id is " << eventid << " and file path is " << path <<
            " and video resolution is " << imgsize);
    }
    else
    {
        thumbnail = fileid.asString() + "_" + imgsize.asString() + ".jpg";
    }

    //删除原始音视频帧文件
    memset(cmd, 0, size);
    snprintf(cmd, size, "rm -f %s", path.asCString());
    system(cmd);

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

    strVideoPath = strFilePath + ".h264";
    if ((videoFd = fopen(strVideoPath.c_str(), "wb")) == NULL)
    {
        LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, can not open video file, path is " << strVideoPath);
        return false;
    }

    strAudioPath = strFilePath + ".aac";
    if ((audioFd = fopen(strAudioPath.c_str(), "wb")) == NULL)
    {
        LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, can not open audio file, path is " << strAudioPath);
        return false;
    }

    unsigned char frameType;
    while (fread(&frameType, sizeof(frameType), 1, mixedFd) == 1)
    {
        //回退1字节读取视频音频头部数据
        fseek(mixedFd, -1L, SEEK_CUR);

        if (frameType == VIDEO_IFRAME || frameType == VIDEO_PFRAME)
        {
            memset(&m_videoInfo, 0, sizeof(VideoInfo));

            if (fread(&m_videoInfo, sizeof(VideoInfo), 1, mixedFd) != 1)
            {
                LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, read frame info error");
                return false;
            }

            unsigned int len = ntohl(m_videoInfo.uiPacketSize);
            if (len > MAX_BUFFER_SIZE)
            {
                LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, packet size is more than maximum: " << len);
                return false;
            }

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
            if (len > MAX_BUFFER_SIZE)
            {
                LOG_ERROR_RLD("SeparateVideoAndAudioFile failed, packet size is more than maximum: " << len);
                return false;
            }

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
            LOG_ERROR_RLD("SeparateAudioAndAudioFile failed, unknown frame type: " << (unsigned int)frameType);
            return false;
        }
    }

    return true;
}

FileHdrEx::FileHdrEx(const unsigned int uiRunTdNum) :
m_MsgReceiver(new InterProcessHandler(InterProcessHandler::RECEIVE_MODE, "mp4_req", uiRunTdNum)),
m_MsgSender(new InterProcessHandler(InterProcessHandler::SEND_MODE, "mp4_rsp"))
{
    m_MsgReceiver->SetMsgOfReceivedHandler(boost::bind(&FileHdrEx::MsgHandler, this, _1));

}

FileHdrEx::~FileHdrEx()
{

}

bool FileHdrEx::Init()
{
    bool blMsgSenderInit = m_MsgSender->Init();
    bool blMsgReceiverInit = m_MsgReceiver->Init();

    LOG_INFO_RLD("Msg sender init is " << blMsgSenderInit << " receiver init is " << blMsgReceiverInit);

    if (!blMsgReceiverInit || !blMsgSenderInit)
    {
        return false;
    }

    return true;
}

void FileHdrEx::Run()
{
    m_MsgReceiver->RunReceivedMsg(true);
}

void FileHdrEx::MsgHandler(const std::string &strMsg)
{
    Mp4FileHandler mp4hdr;
    std::string strResult;
    mp4hdr.Mp4MsgHandler(strMsg, strResult);

    if (strResult.empty())
    {
        LOG_ERROR_RLD("Mp4 file handler failed and source msg is " << strMsg);
        return;
    }

    m_MsgSender->SendMsg(strResult);

}
