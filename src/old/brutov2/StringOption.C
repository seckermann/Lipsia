/* global includes */
#include <string>

/* local includes */
#include "StringOption.h"
#include "StringRegexpValidator.h"
#include "Logger.h"


using namespace logger;

namespace option
{

StringOption::StringOption(std::string name, std::string shortName, std::string help, std::string shortHelp)
			: IOption(name, shortName, help, shortHelp) {
	addSupportedAttribute("REGEXP");
}

StringOption::~StringOption() {
}

bool StringOption::init() {

	if(!IOption::init()) {
		Logger::getInstance().error("calling IOption::init() from StringOption " + getName() + " returned false");
		return false;
	}

	if(hasAttribute("REGEXP")) {
		StringRegexpValidator* _srv = new StringRegexpValidator();
		_srv->setAttribute("REGEXP", getAttribute("REGEXP"));
		addValidator(_srv);
		Logger::getInstance().debug("added StringRegexpValidator to StringOption " + getName());
	}
	
	return true;
}

};
