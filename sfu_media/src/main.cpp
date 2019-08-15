//#include <websocketpp/config/asio_no_tls_client.hpp>
//#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
// This header pulls in the WebSocket++ abstracted thread support that will
// select between boost::thread and std::thread based on how the build system
// is configured.
#include <websocketpp/common/thread.hpp>
#include <fstream>
#include <iostream>
#include "log4z.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <iostream>
#include <iconv.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

using namespace zsummer::log4z;
typedef struct _audioinfo
{
	size_t nSize;
	unsigned char*  pBuffer;
	_audioinfo() {
		nSize = 0;
	}
}audioinfo;

std::vector<audioinfo*> arrAudioBuf;
std::mutex mux;


/**
* Define a semi-cross platform helper method that waits/sleeps for a bit.
*/
void wait_a_bit() {
#ifdef WIN32
	Sleep(1000);
#else
	sleep(1);
#endif
}

/**
* The telemetry client connects to a WebSocket server and sends a message every
* second containing an integer count. This example can be used as the basis for
* programs where a client connects and pushes data for logging, stress/load
* testing, etc.
*/

#ifdef _WIN32
char* GBKToUTF8(const char* chGBK)
{
	DWORD dWideBufSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)chGBK, -1, NULL, 0);
	wchar_t * pWideBuf = new wchar_t[dWideBufSize];
	wmemset(pWideBuf, 0, dWideBufSize);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)chGBK, -1, pWideBuf, dWideBufSize);

	DWORD dUTF8BufSize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pWideBuf, -1, NULL, 0, NULL, NULL);

	char * pUTF8Buf = new char[dUTF8BufSize];
	memset(pUTF8Buf, 0, dUTF8BufSize);
	WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pWideBuf, -1, pUTF8Buf, dUTF8BufSize, NULL, NULL);

	delete[]pWideBuf;
	return pUTF8Buf;
}
std::string UTF8ToGBK(const std::string& strUTF8)
{
	int nLen = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	unsigned short * wszGBK = new unsigned short[nLen + 1];
	memset(wszGBK, 0, nLen * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, (LPWSTR)wszGBK, nLen);

	nLen = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK = new char[nLen + 1];
	memset(szGBK, 0, nLen + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, nLen, NULL, NULL);

	std::string strTemp(szGBK);
	delete[]szGBK;
	delete[]wszGBK;
	return strTemp;
}
#endif

std::string utf8togbk(char* src, size_t srclen)
{
	iconv_t cd = iconv_open("UTF-8", "GB2312");

	size_t nLen = srclen * 3 / 2;
	char* chgbk = new char[srclen * 3/2];
	//char chgbk[2014];
	iconv(cd, &src, &srclen, &chgbk, &nLen);
	std::string strRet = chgbk;
	printf("chgbk:%s\n", chgbk);
	iconv_close(cd);
	//delete[] chgbk;
	//chgbk = NULL;
	return strRet;
}

class telemetry_client {
public:
	//typedef websocketpp::client<websocketpp::config::asio_client> client;
	typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
	typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> scoped_lock;
	typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;
	typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

	telemetry_client() : m_open(false), m_done(false) {
		// set up access channels to only log interesting things
		m_client.clear_access_channels(websocketpp::log::alevel::all);
		m_client.set_access_channels(websocketpp::log::alevel::connect);
		m_client.set_access_channels(websocketpp::log::alevel::disconnect);
		m_client.set_access_channels(websocketpp::log::alevel::app);

		// Initialize the Asio transport policy
		m_client.init_asio();
		m_client.set_tls_init_handler(bind(&on_tls_init));
		// Bind the handlers we are using
		using websocketpp::lib::placeholders::_1;
		using websocketpp::lib::placeholders::_2;
		using websocketpp::lib::bind;
		m_client.set_open_handler(bind(&telemetry_client::on_open, this, _1));
		m_client.set_close_handler(bind(&telemetry_client::on_close, this, _1));
		m_client.set_fail_handler(bind(&telemetry_client::on_fail, this, _1));
		m_client.set_message_handler(bind(&telemetry_client::on_message, this, _1, _2));
	}

	static context_ptr on_tls_init() {
		// establishes a SSL connection
		context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

		try {
			ctx->set_options(boost::asio::ssl::context::default_workarounds |
				boost::asio::ssl::context::no_sslv2 |
				boost::asio::ssl::context::no_sslv3 |
				boost::asio::ssl::context::single_dh_use);
		}
		catch (std::exception &e) {
			std::cout << "Error in context pointer: " << e.what() << std::endl;
		}
		return ctx;
	}

	int AppendHeader(client::connection_ptr con){
		std::string header_type = "Content-type";
		std::string header_typeval = "audio/wav; samplerate=16000";
		//std::string header_rate = "samplerate";
		//std::string header_rateval = "16000";
		std::string header_key = "ApiKey";
		std::string header_keyval = "0ffec4da189baadb6458550ef2151e2a";
		con->append_header(header_type, header_typeval);
		//con->append_header(header_rate, header_rateval);
		con->append_header(header_key, header_keyval);

		/*header_key = "Upgrade";
		header_keyval = "websocket";
		con->append_header(header_key, header_keyval);

		header_key = "Connection";
		header_keyval = "Upgrade";
		con->append_header(header_key, header_keyval);

		header_key = "Host";
		header_keyval = "api.open.newtranx.com";
		con->append_header(header_key, header_keyval);

		header_key = "Origin";
		header_keyval = "http://api.open.newtranx.com";
		con->append_header(header_key, header_keyval);

		header_key = "Sec-WebSocket-Key";
		header_keyval = "C55JR+oXE3qA3xAby/aQGA==";
		con->append_header(header_key, header_keyval);

		header_key = "Sec-WebSocket-Version";
		header_keyval = "13";
		con->append_header(header_key, header_keyval);*/
		return 0;
	}

	// This method will block until the connection is complete
	void run(const std::string & uri) {
		// Create a new connection to the given URI
		websocketpp::lib::error_code ec;
		client::connection_ptr con = m_client.get_connection(uri, ec);
		
		if (ec) {
			m_client.get_alog().write(websocketpp::log::alevel::app, "Get Connection Error: " + ec.message());
			return;
		}
		
		AppendHeader(con);
		//std::string strHeader = con->get_request_header(header_type);
		//std::cout << "header:" << strHeader << std::endl;
		/*
		GET /speech-api/v3/recognize?from=zh-CN HTTP/1.1
		Upgrade: websocket
		Connection: Upgrade
		Host: api.open.newtranx.com
		Origin: http://api.open.newtranx.com
		Sec-WebSocket-Key: C55JR+oXE3qA3xAby/aQGA==
		Sec-WebSocket-Version: 13
		Content-type: audio/wav; samplerate=16000
		ApiKey: 0ffec4da189baadb6458550ef2151e2a
		*/

		// Grab a handle for this connection so we can talk to it in a thread
		// safe manor after the event loop starts.
		
		// Queue the connection. No DNS queries or network connections will be
		// made until the io_service event loop is run.
		m_client.connect(con);
		m_hdl = con->get_handle();
		// Create a thread to run the ASIO io_service event loop
		websocketpp::lib::thread asio_thread(&client::run, &m_client);

		// Create a thread to run the telemetry loop
		websocketpp::lib::thread telemetry_thread(&telemetry_client::telemetry_loop, this);

		asio_thread.join();
		telemetry_thread.join();
	}

	//************************************
	// Method:    on_message
	// FullName:  telemetry_client::on_message
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: websocketpp::connection_hdl
	// Parameter: message_ptr msg
	//************************************
	void on_message(websocketpp::connection_hdl, message_ptr msg)
	{
		std::cout << "======>>>>> on_message called with hdl: " << m_hdl.lock().get() << " and message: " << msg->get_payload() << std::endl;
		//std::cout << utf8togbk(msg->get_payload().c_str(), msg->get_payload().length()).c_str() << std::endl;
		//std::cout << UTF8ToGBK(msg->get_payload().c_str()) << std::endl;
		
		//websocketpp::lib::error_code ec;
	
		//m_client.send(m_hdl, msg->get_payload(), msg->get_opcode(), ec);
		//if (ec) {
		//	std::cout << "Echo failed because: " << ec.message() << std::endl;
		//}
	}

	// The open handler will signal that we are ready to start sending telemetry
	//************************************
	// Method:    on_open
	// FullName:  telemetry_client::on_open
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: websocketpp::connection_hdl
	//************************************
	void on_open(websocketpp::connection_hdl) {
		m_client.get_alog().write(websocketpp::log::alevel::app, "Connection opened, starting telemetry!");

		scoped_lock guard(m_lock);
		m_open = true;
	}

	// The close handler will signal that we should stop sending telemetry
	//************************************
	// Method:    on_close
	// FullName:  telemetry_client::on_close
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: websocketpp::connection_hdl
	//************************************
	void on_close(websocketpp::connection_hdl) {
		m_client.get_alog().write(websocketpp::log::alevel::app, "Connection closed, stopping telemetry!");

		scoped_lock guard(m_lock);
		m_done = true;
	}

	// The fail handler will signal that we should stop sending telemetry
	//************************************
	// Method:    on_fail
	// FullName:  telemetry_client::on_fail
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: websocketpp::connection_hdl
	//************************************
	void on_fail(websocketpp::connection_hdl) {
		m_client.get_alog().write(websocketpp::log::alevel::app, "Connection failed, stopping telemetry!");

		scoped_lock guard(m_lock);
		m_done = true;
	}

	int WebSocketSend(unsigned char* buf, size_t si)
	{
		websocketpp::lib::error_code ec;
		int ret = si;
		int sen = 0;
		while (ret > 0)
		{
			if (ret >= 2730)
			{
				unsigned char *buffer = new unsigned char[2730];    // allocate memory for a buffer of appropriate dimension
				memcpy(buffer, buf + sen, 2730);
				//std::string strtmp = (char*)buffer;
				m_client.send(m_hdl, buffer, 2730, websocketpp::frame::opcode::BINARY, ec);
				if (ec)
					m_client.get_alog().write(websocketpp::log::alevel::app, "Send Error: " + ec.message());
				delete[] buffer;
				buffer = NULL;
				ret -= 2730;
				sen += 2730;
				std::cout << "get:" << sen << " sizeof:" << sizeof(buffer) << " size:" << ret << std::endl;
			}
			else
			{
				unsigned char *buffer = new unsigned char[ret];    // allocate memory for a buffer of appropriate dimension
				memcpy(buffer, buf + sen, ret);
				m_client.send(m_hdl, buffer, ret, websocketpp::frame::opcode::BINARY, ec);
				if (ec)
					m_client.get_alog().write(websocketpp::log::alevel::app, "Send Error: " + ec.message());
				delete[] buffer;
				buffer = NULL;
				ret -= ret;
				sen += ret;
				std::cout << "get:" << sen << " sizeof:" << sizeof(buffer) << " size:" << ret << std::endl;
			}
			usleep(50000);
		}
		return 0;
	}
	//************************************
	// Method:    telemetry_loop
	// FullName:  telemetry_client::telemetry_loop
	// Access:    public 
	// Returns:   void
	// Qualifier:
	//************************************
	void telemetry_loop() {
		uint64_t count = 0;
		std::stringstream val;
		websocketpp::lib::error_code ec;

		while (1) {
			bool wait = false;

			{
				scoped_lock guard(m_lock);
				// If the connection has been closed, stop generating telemetry
				if (m_done) 
				{
					break;
				}

				// If the connection hasn't been opened yet wait a bit and retry
				if (!m_open) {
					wait = true;
				}
			}

			if (wait) {
				wait_a_bit();
				continue;
			}
			count++;

#if 0
			val.str("");
			val << "count is " << count++;
			m_client.get_alog().write(websocketpp::log::alevel::app, val.str());
#endif

//#define SENDFILE

#ifdef SENDFILE
			std::ifstream ifs;
			ifs.open("test1.wav", std::ios::binary);
#if 0
			FILE* fp = fopen("output.wav", "rb");
#endif
			if (ifs.is_open())
			{
				ifs.seekg(0, std::ios::end);    // go to the end  
				size_t length = ifs.tellg();           // report location (this is the length)  
				ifs.seekg(0, std::ios::beg);    // go back to the beginning
#if 0
				fseek(fp, 0, SEEK_END);
				size_t len = ftell(fp);
				fseek(fp, 0, SEEK_SET);
#endif	
				int ret = length;
				while( ret > 0)
				{
					//Sleep(100);
					//continue;
#if 0
					if (ret >= 2730)
					{
						unsigned char *buffer = new unsigned char[2730];    // allocate memory for a buffer of appropriate dimension
						size_t gcou = fread(buffer, 1, 2730, fp);
						m_client.send(m_hdl, (char*)buffer, websocketpp::frame::opcode::BINARY, ec);
						if (ec)
							m_client.get_alog().write(websocketpp::log::alevel::app, "Send Error: " + ec.message());
						delete[] buffer;
						buffer = NULL;
						ret -= 2730;
						std::cout << "get:" << gcou << " size:" << ret << std::endl;
					}
#else
					if (ret >= 2730)
					{
						unsigned char *buffer = new unsigned char[2730];    // allocate memory for a buffer of appropriate dimension
						size_t gcou = ifs.read((char*)buffer, 2730).gcount();
						//std::string strtmp = (char*)buffer;
						m_client.send(m_hdl, buffer, gcou, websocketpp::frame::opcode::BINARY, ec);
						if (ec)
							m_client.get_alog().write(websocketpp::log::alevel::app, "Send Error: " + ec.message());
						delete[] buffer;
						buffer = NULL;
						ret -= 2730;
						//std::cout << "get:" << gcou  << " sizeof:" << sizeof(buffer) << " size:" << ret << std::endl;
					}
					else
					{
						unsigned char *buffer = new unsigned char[ret];    // allocate memory for a buffer of appropriate dimension
						size_t gcou = ifs.read((char*)buffer, ret).gcount();
						m_client.send(m_hdl, buffer, gcou, websocketpp::frame::opcode::BINARY, ec);
						if (ec)
							m_client.get_alog().write(websocketpp::log::alevel::app, "Send Error: " + ec.message());
						delete[] buffer;
						buffer = NULL;
						ret -= 2730;
						std::cout << "get:" << gcou << " sizeof:" << sizeof(buffer) << " size:" << ret << std::endl;
					}
#endif
					usleep(50000);
				}
				ifs.close();
				//m_client.send(m_hdl, buffer, websocketpp::frame::opcode::binary, ec);
				//delete[] buffer;
			}
			
#else
			if (!arrAudioBuf.empty())
			{
				mux.lock();
				audioinfo* pai = arrAudioBuf[0];
				arrAudioBuf.erase(arrAudioBuf.begin());
				mux.unlock();

				WebSocketSend(pai->pBuffer, pai->nSize);
				delete[] pai->pBuffer;
				pai->pBuffer = NULL;
				delete pai;
				pai = NULL;
				//Sleep(10);
				//m_client.send(m_hdl, "EOF", websocketpp::frame::opcode::text, ec);
			}
#endif

#ifdef SENDFILE
			usleep(10000);
			m_client.send(m_hdl, "EOF", websocketpp::frame::opcode::text, ec);
#endif
			// The most likely error that we will get is that the connection is
			// not in the right state. Usually this means we tried to send a
			// message to a connection that was closed or in the process of
			// closing. While many errors here can be easily recovered from,
			// in this simple example, we'll stop the telemetry loop.
			if (ec) {
				m_client.get_alog().write(websocketpp::log::alevel::app, "Send Error: " + ec.message());
				break;
			}

			wait_a_bit();

			usleep(10000);
		}
	}
private:
	client m_client;
	websocketpp::connection_hdl m_hdl;
	websocketpp::lib::mutex m_lock;
	bool m_open;
	bool m_done;
};


#ifdef _WIN32
//************************************
// Method:    OpenAudioDevice
// FullName:  OpenAudioDevice
// Access:    public 
// Returns:   void
// Qualifier: 打开麦克风录音
//************************************
void OpenAudioDevice()
{
	HANDLE          wait;
	HWAVEIN hWaveIn;  //输入设备
	WAVEFORMATEX waveform; //采集音频的格式，结构体
	BYTE *pBuffer1;//采集音频时的数据缓存
	WAVEHDR wHdr1; //采集音频时包含数据缓存的结构体
	waveform.wFormatTag = WAVE_FORMAT_PCM;//声音格式为PCM
	//waveform.nSamplesPerSec = 8000;//采样率，16000次/秒
	waveform.nSamplesPerSec = 16000;//采样率，16000次/秒
	waveform.wBitsPerSample = 16;//采样比特，16bits/次
	waveform.nChannels = 1;//采样声道数，2声道
	waveform.nAvgBytesPerSec = 48000;//每秒的数据率，就是每秒能采集多少字节的数据
	waveform.nBlockAlign = 2;//一个块的大小，采样bit的字节数乘以声道数
	waveform.cbSize = 0;//一般为0

	wait = CreateEvent(NULL, 0, 0, NULL);
	//使用waveInOpen函数开启音频采集
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);

	//建立两个数组（这里可以建立多个数组）用来缓冲音频数据
	DWORD bufsize = 1024 * 100;//每次开辟10k的缓存存储录音数据
	int i = 20;
	FILE * pf;
	fopen_s(&pf, "test.wav", "wb");
	while (i--)//录制20左右秒声音，结合音频解码和网络传输可以修改为实时录音播放的机制以实现对讲功能
	{
		pBuffer1 = new BYTE[bufsize];
		wHdr1.lpData = (LPSTR)pBuffer1;
		wHdr1.dwBufferLength = bufsize;
		wHdr1.dwBytesRecorded = 0;
		wHdr1.dwUser = 0;
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1;
		waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//准备一个波形数据块头用于录音
		waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//指定波形数据块为录音输入缓存
		waveInStart(hWaveIn);//开始录音
		Sleep(1000);//等待声音录制1s
		waveInReset(hWaveIn);//停止录音

		audioinfo * pai = new audioinfo;
		pai->nSize = wHdr1.dwBytesRecorded;
		BYTE* pTmp = new BYTE[wHdr1.dwBytesRecorded];
		pai->pBuffer = pTmp;
		mux.lock();
		arrAudioBuf.push_back(pai);
		mux.unlock();
		fwrite(pBuffer1, 1, wHdr1.dwBytesRecorded, pf);
		fflush(pf);
		delete pBuffer1;
		pBuffer1 = NULL;
		printf("%ds  \n", i);
	}
	waveInClose(hWaveIn);
}
#endif

//************************************
// Method:    main
// FullName:  main
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: int argc
// Parameter: char * argv[]
//************************************
int main(int argc, char* argv[]) {
	telemetry_client c;
	ILog4zManager::getRef().setLoggerPath(LOG4Z_MAIN_LOGGER_ID, "./media_log");
	ILog4zManager::getRef().start();
	ILog4zManager::getRef().setLoggerLevel(LOG4Z_MAIN_LOGGER_ID,LOG_LEVEL_TRACE);

	//LOGD: LOG WITH level LOG_DEBUG
	//LOGI: LOG WITH level LOG_INFO
	
	//begin test stream log input....
	LOGT("stream input *** " << "LOGT LOGT LOGT LOGT" << " *** ");
	LOGD("stream input *** " << "LOGD LOGD LOGD LOGD" << " *** ");
	LOGI("stream input *** " << "LOGI LOGI LOGI LOGI" << " *** ");
	LOGW("stream input *** " << "LOGW LOGW LOGW LOGW" << " *** ");
	LOGE("stream input *** " << "LOGE LOGE LOGE LOGE" << " *** ");
	LOGA("stream input *** " << "LOGA LOGA LOGA LOGA" << " *** ");
	LOGF("stream input *** " << "LOGF LOGF LOGF LOGF" << " *** ");

	//std::string header = "{ \'Content-type\': \'audio/wav; samplerate=16000\', \'ApiKey\' : \'0ffec4da189baadb6458550ef2151e2a\' }";
	
	//std::string params = "?from=zh-CN";
	//std::string url = "wss://api.open.newtranx.com/speech-api/v3/recognize" + params;
	
	//websocket.connect(url, header = header)
	//std::string uri = "wss://39.106.13.209:3306/ws";
	std::string url = "ws://39.106.13.209:9002";
	std::string uri = url;

	if (argc == 2) {
		uri = argv[1];
	}
	//std::thread ti(OpenAudioDevice);
	c.run(uri);
}
