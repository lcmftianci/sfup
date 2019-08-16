#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>

#include <fstream>
#include <iostream>
#include <set>
#include <streambuf>
#include <string>
#include "filenameio.h"
#include "log4z.h"
#include "configmanager.h"
#include <map>

using namespace zsummer::log4z;
/**
 * The telemetry server accepts connections and sends a message every second to
 * each client containing an integer count. This example can be used as the
 * basis for programs that expose a stream of telemetry data for logging,
 * dashboards, etc.
 *
 * This example uses the timer based concurrency method and is self contained
 * and singled threaded. Refer to telemetry client for an example of a similar
 * telemetry setup using threads rather than timers.
 *
 * This example also includes an example simple HTTP server that serves a web
 * dashboard displaying the count. This simple design is suitable for use 
 * delivering a small number of files to a small number of clients. It is ideal
 * for cases like embedded dashboards that don't want the complexity of an extra
 * HTTP server to serve static files.
 *
 * This design *will* fall over under high traffic or DoS conditions. In such
 * cases you are much better off proxying to a real HTTP server for the http
 * requests.
 */
class telemetry_server {
public:
    typedef websocketpp::connection_hdl connection_hdl;
    typedef websocketpp::server<websocketpp::config::asio> server;
    typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
    enum client_type{
        client_media,
        client_camera
    };
    typedef struct _client_hdl{
	std::string m_name;
	int         m_inx;
	client_type m_type;
	_client_hdl(){
		m_name = "";
		m_inx = 0;
		m_type = client_camera;
	}
    }client_hdl;

    telemetry_server() : m_count(0) {
        // set up access channels to only log interesting things
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
        m_endpoint.set_access_channels(websocketpp::log::alevel::app);

        // Initialize the Asio transport policy
        m_endpoint.init_asio();

        // Bind the handlers we are using
        using websocketpp::lib::placeholders::_1;
	using websocketpp::lib::placeholders::_2;
        using websocketpp::lib::bind;
        m_endpoint.set_open_handler(bind(&telemetry_server::on_open,this,_1));
        m_endpoint.set_close_handler(bind(&telemetry_server::on_close,this,_1));
        m_endpoint.set_http_handler(bind(&telemetry_server::on_http,this,_1));
	m_endpoint.set_message_handler(bind(&telemetry_server::on_message, this, _1, _2));
    }

    void on_message(websocketpp::connection_hdl hdl, message_ptr msg)
    {
	//m_connections.find(hdl);
	//解析msg, 判断发送给谁
   	std::cout << "======>>>>> on_message called with hdl: " << hdl.lock().get() << " and message: " << msg->get_payload() << std::endl;
    }

    void run(std::string docroot, uint16_t port) {
        std::stringstream ss;
        ss << "Running telemetry server on port "<< port <<" using docroot=" << docroot;
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,ss.str());
        
        m_docroot = docroot;
        
        // listen on specified port
        m_endpoint.listen(port);

        // Start the server accept loop
        m_endpoint.start_accept();

        // Set the initial timer to start telemetry
        set_timer();

        // Start the ASIO io_service run loop
        try {
            m_endpoint.run();
        } catch (websocketpp::exception const & e) {
            std::cout << e.what() << std::endl;
        }
    }

    void set_timer() {
        m_timer = m_endpoint.set_timer(
            1000,
            websocketpp::lib::bind(
                &telemetry_server::on_timer,
                this,
                websocketpp::lib::placeholders::_1
            )
        );
    }

    void on_timer(websocketpp::lib::error_code const & ec) {
        if (ec) {
            // there was an error, stop telemetry
            m_endpoint.get_alog().write(websocketpp::log::alevel::app, "Timer Error: "+ec.message());
            return;
        }
        
        std::stringstream val;
        //LOGI("==> count is " << m_count++);
        
        // Broadcast count to all connections
        con_list::iterator it;
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
            m_endpoint.send(*it,val.str(),websocketpp::frame::opcode::text);
        }
        
        // set timer for next telemetry check
        set_timer();
    }

    void on_http(connection_hdl hdl) {
        // Upgrade our connection handle to a full connection_ptr
        server::connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
    
        std::ifstream file;
        std::string filename = con->get_resource();
        std::string response;
    
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,
            "http request1: "+filename);
    
        if (filename == "/") {
            filename = m_docroot+"index.html";
        } else {
            filename = m_docroot+filename.substr(1);
        }
        
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,
            "http request2: "+filename);
    
        file.open(filename.c_str(), std::ios::in);
        if (!file) {
            // 404 error
            std::stringstream ss;
        
            ss << "<!doctype html><html><head>"
               << "<title>Error 404 (Resource not found)</title><body>"
               << "<h1>Error 404</h1>"
               << "<p>The requested URL " << filename << " was not found on this server.</p>"
               << "</body></head></html>";
        
            con->set_body(ss.str());
            con->set_status(websocketpp::http::status_code::not_found);
            return;
        }
    
        file.seekg(0, std::ios::end);
        response.reserve(file.tellg());
        file.seekg(0, std::ios::beg);
    
        response.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
        con->set_body(response);
        con->set_status(websocketpp::http::status_code::ok);
    }

    void on_open(connection_hdl hdl) {
        m_connections.insert(hdl);
        //m_arrConnections.push_back(hdl);
	client_hdl chdl;
	m_mapConnections.insert(std::pair<connection_hdl, client_hdl>(hdl, chdl));
	//m_mapConnections[hdl] = chdl;
        LOGD("onopen ==> client count is " << m_mapConnections.size());
    }

    void on_close(connection_hdl hdl) {
        m_connections.erase(hdl);
	m_mapConnections.erase(hdl);
        LOGD("onclose ==> client count is " << m_mapConnections.size());
    }
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
    server m_endpoint;
    con_list m_connections;
    //std::vector<connection_hdl> m_arrConnections;
    std::map<websocketpp::connection_hdl, client_hdl, std::owner_less<connection_hdl>> m_mapConnections;
    server::timer_ptr m_timer;
    
    std::string m_docroot;
    
    // Telemetry data
    uint64_t m_count;
};

int main(int argc, char* argv[]) {
    telemetry_server s;

    std::string docroot;
    uint16_t port = 9002;
    ILog4zManager::getRef().setLoggerPath(LOG4Z_MAIN_LOGGER_ID, "./signal_log");
    ILog4zManager::getRef().start();
    ILog4zManager::getRef().setLoggerLevel(LOG4Z_MAIN_LOGGER_ID,LOG_LEVEL_TRACE);
    LOGT("stream input *** " << "LOGT LOGT LOGT LOGT" << " *** ");
    LOGD("stream input *** " << "LOGD LOGD LOGD LOGD" << " *** ");
    LOGI("stream input *** " << "LOGI LOGI LOGI LOGI" << " *** ");
    LOGW("stream input *** " << "LOGW LOGW LOGW LOGW" << " *** ");
    
    //read config.json
    sfup::sfuputil::ConfigManager* cm = new sfup::sfuputil::ConfigManager();
    sfup::sfuputil::FileNameIO* fni = new sfup::sfuputil::FileNameIO();
    if(fni->IsFileExist("sfu_signal.json")){
	LOGI("signal config exist");
	std::tuple<sfup::sfuputil::rettype, std::string, int> tuipport = cm->SignalConfigParser("sfu_signal.json");
	if(sfup::sfuputil::ret_ok == std::get<0>(tuipport))
	{
		docroot = std::get<1>(tuipport);
		port = std::get<2>(tuipport);
	}
	else
	{
		LOGE("read ip and port failed!");
	}
    }
    else{
       LOGE("signal config not exist");
       LOGE("You Can Usage: telemetry_server [documentroot] [port]");
       return -1;
    }

    if(argc > 2)
    {
	//std::tuple<int, int, vector<int>> tupleTest(1, 4, { 5,6,7,8 });
	std::tuple<std::string, std::string, std::string, int> scport("signal-ip", "signal-port", argv[1], atoi(argv[2]));
	cm->SignalConfigInserter("sfu_signal.json", scport);
	docroot = argv[1];
	port = atoi(argv[2]);
    }
    LOGI("==>> port: " << port << " ip: " << docroot);

    //LOGE("stream input *** " << "LOGE LOGE LOGE LOGE" << " *** ");
#if 0
    if (argc == 1) {
        LOGE("Usage: telemetry_server [documentroot] [port]");
       //std::cout << "Usage: telemetry_server [documentroot] [port]" << std::endl;
        return 1;
    }
    
    if (argc >= 2) {
        docroot = std::string(argv[1]);
    }
   
    if (argc >= 3) {
        int i = atoi(argv[2]);
        if (i <= 0 || i > 65535) {
            std::cout << "invalid port" << std::endl;
            return 1;
        }
        
        port = uint16_t(i);
    }
#endif     

    s.run(docroot, port);
    return 0;
}
