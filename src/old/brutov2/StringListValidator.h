#ifndef _STRINGLISTVALIDATOR_H_
#define _STRINGLISTVALIDATOR_H_

/* global includes */
#include <string>

/* local includes */
#include "IValidator.h"
#include "IOption.h"

namespace option
{

class StringListValidator : public IValidator
{

public:
	StringListValidator();
	virtual ~StringListValidator();

	virtual bool validate( std::string value );

};

};

#endif /*_STRINGLISTVALIDATOR_H_*/
