/* global includes */
#include <string>
#include <sstream>

/* local includes */
#include "DoubleRangeValidator.h"


using namespace std;


namespace option
{

DoubleRangeValidator::DoubleRangeValidator() {
}

DoubleRangeValidator::~DoubleRangeValidator() {
}

bool DoubleRangeValidator::validate(std::string value) {
	
	int iValue;

	// see if the thing is an double at all
	stringstream _valss(value);
	if((_valss >> iValue).fail()) {
		return false;
	}	

	// test for minimum (if attribute is present)
	if(hasAttribute("MINIMUM")) {
		double _min;
		stringstream _minss(getAttribute("MINIMUM"));
		if((_minss >> _min).fail()) { return false; }
		if(iValue < _min) { return false; }
	}
			
	// test for maximum (if attribute is present)
	if(hasAttribute("MAXIMUM")) {
		double _max;
		stringstream _maxss(getAttribute("MAXIMUM"));
		if((_maxss >> _max).fail()) { return false; }
		if(iValue > _max) { return false; }
	}

	return true;
}


};
