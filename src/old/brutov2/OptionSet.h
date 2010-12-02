#ifndef _OPTIONSET_H_
#define _OPTIONSET_H_

/* global includes */
#include <string>
#include <list>
#include <vector>

/* local includes */
#include "IOption.h"
#include "OptionValues.h"


namespace option
{

class OptionSet
{
public:
	OptionSet();
	virtual ~OptionSet();

	virtual bool addOption( IOption *option );
	virtual bool parse( std::vector<std::string> optionString, OptionValues &optionValues );

	virtual IOption *findByName( std::string optionName );
	virtual IOption *findByShortName( std::string optionShortName );

	virtual void printHelp();
	virtual void printExtendedHelp();


private:
	std::list<IOption *> options;
};

};

#endif /*_OPTIONSET_H_*/
