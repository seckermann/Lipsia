#ifndef _DOUBLERANGEVALIDATOR_H_
#define _DOUBLERANGEVALIDATOR_H_

/* global includes */
#include <string>

/* local includes */
#include "IValidator.h"


namespace option
{

class DoubleRangeValidator : public IValidator {
public:
	DoubleRangeValidator();
	virtual ~DoubleRangeValidator();

	virtual bool validate(std::string value);
};

};

#endif /*_DOUBLERANGEVALIDATOR_H_*/
