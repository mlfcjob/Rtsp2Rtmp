#pragma once
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

}


class RtspToRtmp
{
public:
	bool Init(std::string rtspUrl, std::string rtmpUrl);
	int Start();
private:
	std::string m_strRtspUrl;
	std::string m_strRtmpUrl;
	AVFormatContext *m_pRtspAVFormatContext;
	AVFormatContext *m_pRtmpAVFormatContext;
	AVStream		*m_pRtmpAVStream;
	int				m_nRet;
	int				m_nVideoIndex;
	int				m_nAudioIndex;
	char			errBuf[1024];
};

