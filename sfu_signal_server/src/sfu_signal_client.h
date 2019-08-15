#ifndef _SFU_SIGNAL_CLIENT_
#define _SFU_SIGNAL_CLIENT_

/*
 *客户端类：负责管理每个客户端连接信息，区分多媒体服务器还是常规客户端
 *
 *
 * */

#include <iostream>

namespace sfu{
	namespace sfusignal{
		enum cltype{
			cl_media,
			cl_web,
			cl_wcl,
			cl_acl
		};
		
		class SfuSignalClient{
			public:
				SfuSignalClient();

				virtual ~SfuSignalClient();	

			private:
				std::string m_cname;		//客户端名称
				cltype	    m_clt;		//客户端类型	
		};	
	}
}

#endif
