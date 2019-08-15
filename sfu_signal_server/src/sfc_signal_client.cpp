#ifndef _SFU_SIGNAL_CLIENT_
#define _SFU_SIGNAL_CLIENT_


namespace sfu{
	namespace sfusinal{
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
