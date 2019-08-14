#ifndef _FILE_NAME_IO_H_
#define _FILE_NAME_IO_H_

#include <iostream>

namespace sfup{
	namespace sfuputil{
		class FileNameIO{
                	public:
				FileNameIO();
				virtual~FileNameIO();
				bool IsFileExist(std::string strPath);
		};
	}
}

#endif
