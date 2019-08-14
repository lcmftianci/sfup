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
				
rettype ConfigManager::SignalConfigParser(std::string strPath){

	return ret_ok;
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
       	 	LOGW("Open file error!");
    		//LOGD("stream input *** " << "LOGD LOGD LOGD LOGD" << " *** ");
   		//LOGI("stream input *** " << "LOGI LOGI LOGI LOGI" << " *** ");
    		//LOGW("stream input *** " << "LOGW LOGW LOGW LOGW" << " *** ");
   	}
    	f << root.toStyledString(); //转换为json格式并存到文件流
    	f.close();
	return ret_ok;
}

rettype ConfigManager::SignalConfigSaver(std::string strPath, std::tuple<std::string, std::string, std::string> kv){
	return ret_ok;
}
