#ifndef _WEB_RTC_CONNECTION_H__
#define _WEB_RTC_CONNECTION_H_

//负责与webrtc客户端交互与媒体流的转发
/*
	web client --->> createOffer --->> fake webrtc module
	fake webrtc module --->> createAnswer --->> web client
	web client --->> mediastream --->> fake webrtc module
	fake webrtc module --->> send media stream to peer
*/

#include <iostream>

namespace sfup{
	namespace sfupconn{
		class WebrtcConnection{
			public:
				WebrtcConnection();
				virtual ~WebrtcConnection();

				std::string createOffer();
				std::string createAnswer();
				
				
		};
	}

}

#endif
