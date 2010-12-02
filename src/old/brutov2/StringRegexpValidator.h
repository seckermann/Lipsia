#ifndef _STRINGREGEXPVALIDATOR_H_
#define _STRINGREGEXPVALIDATOR_H_

/* global includes */
#include <string>

/* local includes */
#include "IValidator.h"

namespace option
{

class StringRegexpValidator : public IValidator
{

public:
	StringRegexpValidator();
	virtual ~StringRegexpValidator();

	virtual bool validate( std::string value );
};

};

#endif /*_STRINGREGEXPVALIDATOR_H_*/
