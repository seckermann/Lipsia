#ifndef _STRINGLISTOPTION_H_
#define _STRINGLISTOPTION_H_

/* global includes */


/* local includes */
#include "IOption.h"


namespace option
{

class StringListOption : public IOption
{
public:
	StringListOption( std::string name, std::string shortName, std::string help, std::string shortHelp );
	virtual ~StringListOption();

	virtual bool init();
};

};

#endif /*_STRINGLISTOPTION_H_*/
