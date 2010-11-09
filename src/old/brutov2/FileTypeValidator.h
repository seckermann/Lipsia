#ifndef _FILETYPEVALIDATOR_H_
#define _FILETYPEVALIDATOR_H_

/* global includes */
#include <string>

/* local includes */
#include "IValidator.h"

namespace option
{

class FileTypeValidator : public IValidator {

public:
	FileTypeValidator();
	virtual ~FileTypeValidator();

	virtual bool validate(std::string value);
};

};

#endif /*_FILETYPEVALIDATOR_H_*/
