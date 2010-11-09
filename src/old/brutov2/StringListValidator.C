/* global includes */
#include <string>
#include <vector>
#include <algorithm>

/* local includes */
#include "StringListValidator.h"
#include "StringUtilities.h"


using namespace std;
using namespace strutils;


namespace option
{

StringListValidator::StringListValidator() {
}

StringListValidator::~StringListValidator() {
}

bool StringListValidator::validate(std::string value) {
	bool _result = true; // positive thinking
	
	// the 'LIST' attribute is always present - at least when we got this far
	vector<string> _list = StringUtilities::tokenizeString(getAttribute("LIST"), ",");	
	if(find(_list.begin(), _list.end(), value) == _list.end()) {
		_result = false;
	}
		
	return _result;
}

};
