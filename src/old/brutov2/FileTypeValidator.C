/* global includes */
#include <string>

/* local includes */
#include "FileTypeValidator.h"


namespace option
{

FileTypeValidator::FileTypeValidator() {
}

FileTypeValidator::~FileTypeValidator() {
}

bool FileTypeValidator::validate(std::string value) {
	bool _result = true; // positive thinking, man

	if(hasAttribute("TYPE")) {
		// TODO: please implement me :-(
	}
	
	return _result;
}

};
