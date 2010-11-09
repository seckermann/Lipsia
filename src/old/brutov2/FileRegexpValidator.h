#ifndef _FILEREGEXPVALIDATOR_H_
#define _FILEREGEXPVALIDATOR_H_

/* global includes */
#include <string>

/* local includes */
#include "IValidator.h"

namespace option
{

class FileRegexpValidator : public IValidator {

public:
	FileRegexpValidator();
	virtual ~FileRegexpValidator();

	virtual bool validate(std::string value);
};

};

#endif /*_FILEREGEXPVALIDATOR_H_*/
