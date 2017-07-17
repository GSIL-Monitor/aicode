#include <cstdio>
#include <boost/scope_exit.hpp>
#include "MP4Encoder.h"
#include "MP4Converter.h"
#include "LogRLD.h"

bool MP4Converter::Init(const std::string &strVideoFile, const std::string &strAudioFile)
{
    if (NULL == (m_videoFd = fopen(strVideoFile.c_str(), "rb")))
    {
        LOG_ERROR_RLD("Could not open video file!\n");
        return false;
    }

    if (NULL == (m_audioFd = fopen(strAudioFile.c_str(), "rb")))
    {
        LOG_ERROR_RLD("Could not open audio file!\n");
        return false;
    }

    return true;
}

void MP4Converter::Release(const bool blResult)
{
    if (m_videoFd != NULL)
    {
        fclose(m_videoFd);
        m_videoFd = NULL;
    }

    if (m_audioFd != NULL)
    {
        fclose(m_audioFd);
        m_audioFd = NULL;
    }

    m_mp4Encoder.MP4ReleaseFile();

    if (!blResult)
    {
        remove(m_strMp4FilePath.c_str());
    }
}

MP4Converter::MP4Converter()
{
    buf = new uint8_t[MAX_BUFFER_SIZE];
    memset(buf, 0, MAX_BUFFER_SIZE);
}

MP4Converter::~MP4Converter()
{
    //Release();
    if (buf != NULL)
    {
        delete buf;
        buf = NULL;
    }

}

MP4EncoderResult MP4Converter::AddH264Track(const int iWidth, const int iHeight, const int iFrameRate)
{
    unsigned int len = 0;
    if (fread(&len, sizeof(int), 1, m_videoFd) == 1)
    {
        uint64_t pts = 0;
        if (fread(&pts, sizeof(uint64_t), 1, m_videoFd) == 1 && fread(buf, sizeof(uint8_t), len, m_videoFd) == len)
        {
            //unsigned char dta[25];
            //for (int i = 0; i < 25; i++)
            //{
            //    dta[i] = buf[i];
            //}

            return m_mp4Encoder.MP4AddH264Track(buf, len, iWidth, iHeight, iFrameRate);
        }
    }

    LOG_ERROR_RLD("AddH264Track failed");
    return MP4ENCODER_ERROR(MP4ENCODER_E_ADD_VIDEO_TRACK);
}

MP4EncoderResult MP4Converter::AddAACTrack()
{
    unsigned int len = 0;
    if (fread(&len, sizeof(int), 1, m_audioFd) == 1)
    {
        uint64_t pts = 0;
        if (fread(&pts, sizeof(uint64_t), 1, m_audioFd) == 1 && fread(buf, sizeof(uint8_t), len, m_audioFd) == len)
        {
            return m_mp4Encoder.MP4AddAACTrack(buf, len);
        }
    }

    LOG_ERROR_RLD("AddAACTrack failed");
    return MP4ENCODER_ERROR(MP4ENCODER_E_ADD_AUDIO_TRACK);
}

bool MP4Converter::WriteH264Data()
{
    unsigned int len = 0;
    while (fread(&len, sizeof(int), 1, m_videoFd) == 1)
    {
        uint64_t pts = 0;
        if (fread(&pts, sizeof(uint64_t), 1, m_videoFd) == 1 && fread(buf, sizeof(uint8_t), len, m_videoFd) == len)
        {
            return m_mp4Encoder.MP4WriteH264Data(buf, len, pts) == MP4EncoderResult::MP4ENCODER_ENONE ? true : false;
        }
    }

    LOG_ERROR_RLD("WriteH264Data failed");
    return false;
}

bool MP4Converter::WriteAACData()
{
    unsigned int len = 0;
    while (fread(&len, sizeof(int), 1, m_audioFd) == 1)
    {
        if (fread(&len, sizeof(int), 1, m_audioFd) == 1)
        {
            uint64_t pts = 0;
            if (fread(&pts, sizeof(uint64_t), 1, m_audioFd) == 1 && fread(buf, sizeof(uint8_t), len, m_audioFd) == len)
            {
                return m_mp4Encoder.MP4WriteAACData(buf, len, pts) == MP4EncoderResult::MP4ENCODER_ENONE ? true : false;
            }
        }
    }

    LOG_ERROR_RLD("WriteAACData failed");
    return false;
}

bool MP4Converter::ConvertVideo(const std::string &strVideoFile, const std::string &strAudioFile, const int iWidth,
    const int iHeight, const int iFrameRate)
{
    std::string sep = "/video_";
    m_strMp4FilePath = strVideoFile;
    m_strMp4FilePath.replace(m_strMp4FilePath.find_last_of(sep), sep.size(), "/").append(".mp4");

    bool blResult = false;

    BOOST_SCOPE_EXIT(this_, &blResult)
    {
        this_->Release(blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!Init(strVideoFile, strAudioFile))
        {
            LOG_ERROR_RLD("ConvertVideo failed, init error, video file is " << strVideoFile <<
                " and audio file is " << strAudioFile);
            return false;
        }

    if (m_mp4Encoder.MP4CreateFile(m_strMp4FilePath.c_str(), 5) != MP4EncoderResult::MP4ENCODER_ENONE)
    {
        LOG_ERROR_RLD("ConvertVideo failed, create mp4 file error, path is " << m_strMp4FilePath);
        return false;
    }

    if (AddH264Track(iWidth, iHeight, iFrameRate) != MP4EncoderResult::MP4ENCODER_ENONE ||
        AddAACTrack() != MP4EncoderResult::MP4ENCODER_ENONE)
    {
        LOG_ERROR_RLD("ConvertVideo failed, add track error");
        return false;
    }

    if (!WriteH264Data() || !WriteAACData())
    {
        LOG_ERROR_RLD("ConvertVideo failed, add mp4 date error");
        return false;
    }

    blResult = true;
    return blResult;
}
