/* global includes */
#include <string>

/* local includes */
#include "FileOption.h"
#include "FileRegexpValidator.h"
#include "FileTypeValidator.h"
#include "Logger.h"


using namespace std;
using namespace logger;

namespace option
{

FileOption::FileOption(std::string name, std::string shortName, std::string help, std::string shortHelp)
			: IOption(name, shortName, help, shortHelp) {
	addSupportedAttribute("TYPE");
	addSupportedAttribute("REGEXP");
}

FileOption::~FileOption() {
}

bool FileOption::init() {

	if(!IOption::init()) {
		Logger::getInstance().error("calling IOption::init() from FileOption " + getName() + " returned false");
		return false;
	}

	if(hasAttribute("TYPE")) {
		// at the moment we only support 'FILE' and 'DIRECTORY' as types
		string _type = getAttribute("TYPE");
		if(_type.compare("FILE") != 0 && _type.compare("DIRECTORY") != 0) {
			Logger::getInstance().error("in FileOption " + getName() + ": attribute 'TYPE=" + _type + "' is not supported (only 'FILE' and 'DIRECTORY')");
			return false;
		}
		FileTypeValidator* _ftv = new FileTypeValidator();
		_ftv->setAttribute("TYPE", getAttribute("TYPE"));
		addValidator(_ftv);
		Logger::getInstance().debug("added FileTypeValidator to FileOption " + getName());
	}

	if(hasAttribute("REGEXP")) {
		FileRegexpValidator* _frv = new FileRegexpValidator();
		_frv->setAttribute("REGEXP", getAttribute("REGEXP"));
		addValidator(_frv);
		Logger::getInstance().debug("added FileRegexpValidator to FileOption " + getName());
	}
	
	return true;
}

};
