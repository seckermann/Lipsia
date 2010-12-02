#ifndef _INTEGEROPTION_H_
#define _INTEGEROPTION_H_

/* global includes */


/* local includes */
#include "IOption.h"
#include "IValidator.h"


namespace option
{

class IntegerOption : public IOption
{

public:

	IntegerOption( std::string name, std::string shortName, std::string help, std::string shortHelp );
	virtual ~IntegerOption();

protected:
	virtual bool init();
};

};

#endif /*_INTEGEROPTION_H_*/
