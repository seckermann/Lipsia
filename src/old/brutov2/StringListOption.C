/* global includes */
#include <string>

/* local includes */
#include "StringListOption.h"
#include "StringListValidator.h"
#include "Logger.h"


using namespace logger;


namespace option
{

StringListOption::StringListOption(std::string name, std::string shortName, std::string help, std::string shortHelp)
			: IOption(name, shortName, help, shortHelp) {
	addSupportedAttribute("LIST");
}

StringListOption::~StringListOption() {
}

bool StringListOption::init() {
	if(!IOption::init()) {
		Logger::getInstance().error("calling IOption::init() from StringListOption " + getName() + " returned false");
		return false;
	}

	if(!hasAttribute("LIST")) {
		Logger::getInstance().error("StringListOption " + getName() + " doesn't have the required 'LIST' attribute");
		return false;
	}
	
	StringListValidator* _slv = new StringListValidator();
	_slv->setAttribute("LIST", getAttribute("LIST"));
	addValidator(_slv);
	Logger::getInstance().debug("added StringListValidator to StringListOption " + getName());
	
	return true;
}

};
