#ifndef _DOUBLEOPTION_H_
#define _DOUBLEOPTION_H_

/* global includes */


/* local includes */
#include "IOption.h"
#include "IValidator.h"


namespace option
{

class DoubleOption : public IOption {
	
public:

	DoubleOption(std::string name, std::string shortName, std::string help, std::string shortHelp);
	virtual ~DoubleOption();

protected:	
	virtual bool init();
};

};

#endif /*_DOUBLEOPTION_H_*/
