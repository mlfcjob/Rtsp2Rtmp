#include "RtspToRtmp.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

#include <Windows.h>

bool RtspToRtmp::Init(std::string rtspUrl, std::string rtmpUrl)
{
	//av_register_all();
	avformat_network_init();


	m_strRtspUrl = rtspUrl;
	m_strRtmpUrl = rtmpUrl;

	m_pRtspAVFormatContext = NULL;
	m_nRet = 0;

	m_nVideoIndex = -1;
	m_nAudioIndex = -1;
	
	memset(errBuf, 0, 1024);

	// 1. 申请输出流上下文
	m_pRtmpAVFormatContext = NULL;
	avformat_alloc_output_context2(&m_pRtmpAVFormatContext, NULL, "flv", m_strRtmpUrl.c_str());
	if (!m_pRtmpAVFormatContext)
	{
		std::cout << "avformat_alloc_output_contex2 failed!" << std::endl;
		return false;
	}
	std::cout << "[" << m_strRtmpUrl.c_str() << "] avformat_alloc_output_context2 success " << std::endl;


	return true;
}

int RtspToRtmp::Start()
{
	//1.打开输入流
	m_nRet = avformat_open_input(&m_pRtspAVFormatContext, m_strRtspUrl.c_str(), 0, 0);
	if (m_nRet != 0)
	{
		std::cout << "open " << m_strRtspUrl.c_str() << "error!" << std::endl;
		return m_nRet;
	}
	std::cout << "open " << m_strRtspUrl.c_str() << "success !" << std::endl;

	// 2.获得流信息
	m_nRet = avformat_find_stream_info(m_pRtspAVFormatContext, 0);
	if (m_nRet < 0)
	{
		std::cout << "avformat_find_stream_info failed" << std::endl;
		return m_nRet;
	}

	std::cout << "avformat_find_stream_info success" << std::endl;
	av_dump_format(m_pRtspAVFormatContext, 0, m_strRtspUrl.c_str(),0);



	for (int i = 0; i < m_pRtspAVFormatContext->nb_streams; i++)
	{
		if (m_pRtspAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
		{
			m_nVideoIndex = i;
		}

		if (m_pRtspAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) 
		{ 
			m_nAudioIndex = i;
		}
	}

	std::cout << "VideoIndex: " << m_nVideoIndex << std::endl;
	std::cout << "AudioIndex: " << m_nAudioIndex << std::endl;

	//3. 申请输出AVStream(rtmp)
	const AVCodec* outCodec = avcodec_find_decoder(m_pRtspAVFormatContext->streams[m_nVideoIndex]->codecpar->codec_id);
	m_pRtmpAVStream = avformat_new_stream(m_pRtmpAVFormatContext, outCodec);
	if (!m_pRtmpAVStream)
	{
		std::cout << "[rtmp] Failed allocating output stream!" << std::endl;
		return m_nRet;
	}

	std::cout << "[rtmp] avformat_new_stream success!" << std::endl;

	//4. stream信息复制
	AVCodecContext* rtmpOutCodec = avcodec_alloc_context3(outCodec);

	m_nRet = avcodec_parameters_to_context(rtmpOutCodec, m_pRtspAVFormatContext->streams[m_nVideoIndex]->codecpar);
	if (m_nRet < 0)
	{
		std::cout << "[rtmp] avcodec_paramertes_to_context failed " << std::endl;
		return m_nRet;
	}
	std::cout << "[rtmp] avcodec_paramertes_to_context success" << std::endl;

	m_nRet = avcodec_parameters_from_context(m_pRtmpAVStream->codecpar, rtmpOutCodec);
	if (m_nRet < 0)
	{
		std::cout << "[rtmp] avcodec_parameters_from_context failed";
		return m_nRet;
	}


	if (!(m_pRtmpAVFormatContext->flags & AVFMT_NOFILE)) {
		m_nRet = avio_open2(&m_pRtmpAVFormatContext->pb, m_strRtmpUrl.c_str(), AVIO_FLAG_WRITE, NULL, NULL);
		if (m_nRet < 0) {
			av_strerror(m_nRet, errBuf, 1024);
			//std::cout << "avio_open2 failed: " << errBuf << std::endl;
			printf("avio_open2 failed: %s\n", errBuf);
			return m_nRet;
		}
	}


	//5. 写rtmp头信息
	m_nRet = avformat_write_header(m_pRtmpAVFormatContext, NULL);
	if (m_nRet < 0)
	{
		std::cout << "[rtmp] avformat_write_header failed" << std::endl;

		av_strerror(m_nRet, errBuf, 1024);
		printf("errBuf: %s\n", errBuf);
		//std::cout << err << std::endl;
		return m_nRet;
	}

	std::cout << "[rtmp] avformat_write_header success!" << std::endl;

	//6. 数据包读取和写入
	AVPacket pkt;
	while (1)
	{
		AVStream* rtspStream, *rtmpStream;
		m_nRet = av_read_frame(m_pRtspAVFormatContext, &pkt);
		if (m_nRet < 0)
		{
			break;
		}

		rtspStream = m_pRtspAVFormatContext->streams[pkt.stream_index];
		rtmpStream = m_pRtmpAVFormatContext->streams[pkt.stream_index];

		pkt.pts = av_rescale_q_rnd(pkt.pts, rtspStream->time_base, rtmpStream->time_base, AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, rtspStream->time_base, rtmpStream->time_base, AVRounding(AV_ROUND_NEAR_INF |AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, rtspStream->time_base, rtmpStream->time_base);

		pkt.pos = -1;
		m_nRet = av_interleaved_write_frame(m_pRtmpAVFormatContext, &pkt);
		if (m_nRet < 0)
		{
			break;
		}
		av_packet_unref(&pkt);
		//Sleep(40);
	}

	return m_nRet;
}