#ifndef _FILEOPTION_H_
#define _FILEOPTION_H_

/* global includes */


/* local includes */
#include "IOption.h"


namespace option
{

class FileOption : public IOption
{
public:
	FileOption( std::string name, std::string shortName, std::string help, std::string shortHelp );
	virtual ~FileOption();

	virtual bool init();

	friend class OptionSet;
};

};

#endif /*_FILEOPTION_H_*/
