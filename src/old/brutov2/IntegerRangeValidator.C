/* global includes */
#include <string>
#include <sstream>

/* local includes */
#include "IntegerRangeValidator.h"


using namespace std;


namespace option
{

IntegerRangeValidator::IntegerRangeValidator() {
}

IntegerRangeValidator::~IntegerRangeValidator() {
}

bool IntegerRangeValidator::validate(std::string value) {
	
	int iValue;

	// see if the thing is an integer at all
	stringstream _valss(value);
	if((_valss >> iValue).fail()) {
		return false;
	}	

	// test for minimum (if attribute is present)
	if(hasAttribute("MINIMUM")) {
		int _min;
		stringstream _minss(getAttribute("MINIMUM"));
		if((_minss >> _min).fail()) { return false; }
		if(iValue < _min) { return false; }
	}
			
	// test for maximum (if attribute is present)
	if(hasAttribute("MAXIMUM")) {
		int _max;
		stringstream _maxss(getAttribute("MAXIMUM"));
		if((_maxss >> _max).fail()) { return false; }
		if(iValue > _max) { return false; }
	}

	return true;
}


};
