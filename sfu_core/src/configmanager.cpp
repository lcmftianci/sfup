#include "configmanager.h"
#include "json/json.h"
#include "log4z.h"
#include "fstream"

using namespace zsummer::log4z;

using namespace sfup::sfuputil;
using namespace std;

ConfigManager::ConfigManager(){

}



ConfigManager::~ConfigManager(){


}
				
std::tuple<rettype, string, int> ConfigManager::SignalConfigParser(std::string strPath){
	
	Json::Value root;
	Json::Reader reader;
	ifstream ifs(strPath, ios::binary);
	if(!ifs.is_open())
	{
       	 	LOGW("signal config file, Open file error!");
		tuple<rettype, string, int> rettuple(ret_error, "0.0.0.0", 9002);
		return rettuple;
	}

	if (reader.parse(ifs, root))
	{
		tuple<rettype, string, int> rettuple(ret_ok, root["signal_ip"].asString(), root["signal_port"].asInt());
		return rettuple;
	}
	tuple<rettype, string, int> rettuple(ret_error, "0.0.0.0", 9002);
	return rettuple;
}


rettype ConfigManager::SignalConfigInserter(std::string strPath, std::tuple<std::string, std::string, std::string, int> kv){
	Json::Value root; //根节点
	Json::Value node;

	root[get<0>(kv).c_str()] = get<2>(kv).c_str();
	root[get<1>(kv).c_str()] = get<3>(kv);
	//root[get<0>(kv).c_str()] = node;
	
 	fstream f;
    	f.open (strPath, ios::out);
    	if( !f.is_open() ){
       	 	LOGW("signal config file Open file error!");
    		//LOGD("stream input *** " << "LOGD LOGD LOGD LOGD" << " *** ");
   		//LOGI("stream input *** " << "LOGI LOGI LOGI LOGI" << " *** ");
    		//LOGW("stream input *** " << "LOGW LOGW LOGW LOGW" << " *** ");
    		return ret_error;
   	}
    	f << root.toStyledString(); //转换为json格式并存到文件流
    	f.close();
	return ret_ok;
}

rettype ConfigManager::SignalConfigSaver(std::string strPath, std::tuple<std::string, std::string, std::string> kv){
	return ret_ok;
}
