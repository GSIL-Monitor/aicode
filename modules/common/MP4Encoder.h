// MP4Encoder.h



#ifndef _MP4EN_MP4V2_H_
#define _MP4EN_MP4V2_H_
#include "mp4v2/mp4v2.h"


#define MP4ENCODER_ERROR(err) ((MP4EncoderResult)(-(err)))
#define DEFAULT_RECORD_TIME 0U

typedef enum
{
	MP4ENCODER_ENONE = 0,
	MP4ENCODER_E_CREATE_FAIL,
	MP4ENCODER_E_ADD_VIDEO_TRACK,
	MP4ENCODER_E_ADD_AUDIO_TRACK,
	MP4ENCODER_WARN_RECORD_OVER,
	MP4ENCODER_E_WRITE_VIDEO_DATA,
	MP4ENCODER_E_WRITE_AUDIO_DATA,
	MP4ENCODER_E_ALLOC_MEMORY_FAILED,
	MP4ENCODER_E_UNKONOWN
}MP4EncoderResult;

class MP4Encoder
{
public:
	MP4Encoder(void);
	~MP4Encoder(void);
	MP4EncoderResult MP4CreateFile(const char *sFileName,
		unsigned uRecordTime = DEFAULT_RECORD_TIME);
	MP4EncoderResult MP4AddH264Track(const uint8_t *sData, int nSize,
		int nWidth, int nHeight, int nFrameRate = 25);
	MP4EncoderResult MP4AddAACTrack(const uint8_t *sData, int nSize);
	MP4EncoderResult MP4WriteH264Data(uint8_t *sData, int nSize, uint64_t u64PTS);
	MP4EncoderResult MP4WriteAACData(const uint8_t *sData, int nSize,
		uint64_t u64PTS);
	void MP4ReleaseFile();
private:
	
	MP4FileHandle m_hFile;
	bool m_bFirstVideo;
    bool m_bFirstAudio;
    unsigned m_uSecond;
    MP4TrackId m_videoTrack;
    MP4TrackId m_audioTrack;
    uint64_t m_u64VideoPTS;
    uint64_t m_u64AudioPTS;
    uint64_t m_u64FirstPTS;
    uint64_t m_u64LastPTS;
};

#endif