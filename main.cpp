//#include <iostream>
#include "RtspToRtmp.h"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "usage: " << argv[0] << "[in_rtsp_url] [out_rtmp_url]" << endl;
		cout << "eg: " << argv[0] << " rtsp://admin:admin@192.168.1.64 rtmp://192.168.1.102:1935/rtmp_live/test" << endl;
		return -1;
	}
	RtspToRtmp* app = new RtspToRtmp();
	std::string rtsp = argv[1];
	std::string rtmp = argv[2];
	app->Init(rtsp, rtmp);
	app->Start();
	getchar();
	return 0;
}