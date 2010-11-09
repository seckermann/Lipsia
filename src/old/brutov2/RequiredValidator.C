#include "RequiredValidator.h"

namespace option
{

RequiredValidator::RequiredValidator() {
}

RequiredValidator::~RequiredValidator() {
}


bool RequiredValidator::validate(std::string value) {
	return (value.compare("") != 0);
}

};
