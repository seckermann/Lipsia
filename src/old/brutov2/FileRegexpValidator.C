/* global includes */
#include <string>

/* local includes */
#include "FileRegexpValidator.h"


namespace option
{

FileRegexpValidator::FileRegexpValidator() {
}

FileRegexpValidator::~FileRegexpValidator() {
}

bool FileRegexpValidator::validate(std::string value) {
	bool _result = true; // positive thinking, man
	
	if(hasAttribute("REGEXP")) {
		// TODO: yet to implement
		// NOTE: we should have a look at the boost library for this
	}
	
	return _result;
}

};
