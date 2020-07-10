//#include <iostream>
#include "RtspToRtmp.h"

using namespace std;

int main(int argc, char* argv)
{
	RtspToRtmp* app = new RtspToRtmp();
	std::string rtsp = "rtsp://admin:admin123@192.17.1.73";
	rtsp = "rtsp://120.53.12.211/test1.sdp";
	std::string rtmp = "rtmp://192.17.1.202:1935/rtmp_live/test";
	rtmp = "rtmp://120.53.12.211:1935/rtmp_live/test";
	app->Init(rtsp, rtmp);
	app->Start();
	getchar();
	return 0;
}