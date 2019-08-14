#include "filenameio.h"
#include <unistd.h>

using namespace sfup;
using namespace sfuputil;

FileNameIO::FileNameIO(){

}

FileNameIO::~FileNameIO(){

}


bool FileNameIO::IsFileExist(std::string strPath)
{
	if(!access(strPath.c_str(), 0))
		return true;
	return false;
}
