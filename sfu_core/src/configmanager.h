#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include <iostream>
#include <tuple>

namespace sfup{
	namespace sfuputil{

		enum rettype{
			ret_nodef,
			ret_error,
			ret_normal,
			ret_ok,
			ret_signal_error,
			ret_signal_ok
		};

		class ConfigManager{
			public:
				ConfigManager();
				virtual	~ConfigManager();
				
				rettype SignalConfigParser(std::string strPath);
				
				rettype SignalConfigInserter(std::string strPath, std::tuple<std::string, std::string, std::string, int> kv);
				rettype SignalConfigSaver(std::string strPath, std::tuple<std::string, std::string, std::string> kv);
		};
	}
}

#endif
