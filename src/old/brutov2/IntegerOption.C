
/* global includes */
#include <sstream>

/* local includes */
#include "IntegerOption.h"
#include "IValidator.h"
#include "IntegerRangeValidator.h"
#include "Logger.h"


using namespace logger;


namespace option
{

IntegerOption::IntegerOption(std::string name, std::string shortName, std::string help, std::string shortHelp)
			: IOption(name, shortName, help, shortHelp) {
	addSupportedAttribute("MINIMUM");
	addSupportedAttribute("MAXIMUM");
}

IntegerOption::~IntegerOption() {
}

bool IntegerOption::init() {

	if(!IOption::init()) {
		Logger::getInstance().error("calling IOption::init() from IntegerOption " + getName() + " returned false");
		return false;
	}

	// the following stuff is not really optimized - but
	// it's far away from being a performance hit and can
	// therefore stay this way for readability reasons
	
	if(hasAttribute("MINIMUM") && hasAttribute("MAXIMUM")) {
		std::stringstream _minss(getAttribute("MINIMUM"));
		int _min;
		_minss >> _min;
		std::stringstream _maxss(getAttribute("MAXIMUM"));
		int _max;
		_maxss >> _max;
		if(_max < _min) {
			std::stringstream message;
			message << "in IntegerOption " << getName() << " MAXIMUM(" << _max << ") < MINIMUM(" << _min << ")";
			Logger::getInstance().error(message.str());
			return false;
		}
	}

	IntegerRangeValidator* _irv = new IntegerRangeValidator();
	if(hasAttribute("MINIMUM")) {
		_irv->setAttribute("MINIMUM", getAttribute("MINIMUM"));
	}
	if(hasAttribute("MAXIMUM")) {
		_irv->setAttribute("MAXIMUM", getAttribute("MAXIMUM"));
	}
	addValidator(_irv);
	
	return true;
}

};
