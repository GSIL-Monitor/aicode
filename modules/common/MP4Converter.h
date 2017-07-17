#ifndef _MP4_CONVERTER_H_
#define _MP4_CONVERTER_H_

#include <string>

class MP4Converter
{
public:
    static const int MAX_BUFFER_SIZE = 500 * 1024;

    MP4Converter();
    ~MP4Converter();

    bool Init(const std::string &strVideoFile, const std::string &strAudioFile);
    void Release(const bool blResult);
    MP4EncoderResult AddH264Track(const int iWidth, const int iHeight, const int iFrameRate);
    MP4EncoderResult AddAACTrack();
    bool WriteH264Data();
    bool WriteAACData();
    bool ConvertVideo(const std::string &strVideoFile, const std::string &strAudioFile, const int iWidth,
        const int iHeight, const int iFrameRate);

private:
    MP4Encoder m_mp4Encoder;

    FILE *m_videoFd;
    FILE *m_audioFd;

    std::string m_strMp4FilePath;

    uint8_t *buf;

};


#endif // !_MP4_CONVERTER_H_





