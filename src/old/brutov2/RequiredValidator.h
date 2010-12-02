#ifndef _REQUIREDVALIDATOR_H_
#define _REQUIREDVALIDATOR_H_


/* global includes */
#include <string>

/* local includes */
#include "IValidator.h"


namespace option
{

class RequiredValidator : public IValidator
{
public:
	RequiredValidator();
	virtual ~RequiredValidator();

	virtual bool validate( std::string value );
};

};

#endif /*_REQUIREDVALIDATOR_H_*/
