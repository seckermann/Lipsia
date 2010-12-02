#ifndef _STRINGOPTION_H_
#define _STRINGOPTION_H_

/* global includes */


/* local includes */
#include "IOption.h"


namespace option
{

class StringOption : public IOption
{
public:
	StringOption( std::string name, std::string shortName, std::string help, std::string shortHelp );
	virtual ~StringOption();

	virtual bool init();

	friend class OptionSet;
};

};

#endif /*_STRINGOPTION_H_*/
