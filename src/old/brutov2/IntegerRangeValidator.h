#ifndef _INTEGERRANGEVALIDATOR_H_
#define _INTEGERRANGEVALIDATOR_H_

/* global includes */
#include <string>

/* local includes */
#include "IValidator.h"


namespace option
{

class IntegerRangeValidator : public IValidator {
public:
	IntegerRangeValidator();
	virtual ~IntegerRangeValidator();

	virtual bool validate(std::string value);
};

};

#endif /*_INTEGERRANGEVALIDATOR_H_*/
