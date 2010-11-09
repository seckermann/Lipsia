
/* global includes */
#include <sstream>

/* local includes */
#include "DoubleOption.h"
#include "IValidator.h"
#include "DoubleRangeValidator.h"
#include "Logger.h"


using namespace logger;


namespace option
{

DoubleOption::DoubleOption(std::string name, std::string shortName, std::string help, std::string shortHelp)
			: IOption(name, shortName, help, shortHelp) {
	addSupportedAttribute("MINIMUM");
	addSupportedAttribute("MAXIMUM");
}

DoubleOption::~DoubleOption() {
}

bool DoubleOption::init() {

	if(!IOption::init()) {
		Logger::getInstance().error("calling IOption::init() from DoubleOption " + getName() + " returned false");
		return false;
	}

	// the following stuff is not really optimized - but
	// it's far away from being a performance hit and can
	// therefore stay this way for readability reasons
	
	if(hasAttribute("MINIMUM") && hasAttribute("MAXIMUM")) {
		std::stringstream _minss(getAttribute("MINIMUM"));
		double _min;
		_minss >> _min;
		std::stringstream _maxss(getAttribute("MAXIMUM"));
		double _max;
		_maxss >> _max;
		if(_max < _min) {
			std::stringstream message;
			message << "in DoubleOption " << getName() << " MAXIMUM(" << _max << ") < MINIMUM(" << _min << ")";
			Logger::getInstance().error(message.str());
			return false;
		}
	}

	DoubleRangeValidator* _drv = new DoubleRangeValidator();
	if(hasAttribute("MINIMUM")) {
		_drv->setAttribute("MINIMUM", getAttribute("MINIMUM"));
	}
	if(hasAttribute("MAXIMUM")) {
		_drv->setAttribute("MAXIMUM", getAttribute("MAXIMUM"));
	}
	addValidator(_drv);
	
	return true;
}

};
