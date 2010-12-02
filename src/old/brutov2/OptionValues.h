#ifndef _OPTIONVALUES_H_
#define _OPTIONVALUES_H_

/* global includes */
#include <string>
#include <map>

/* local includes */



namespace option
{

class OptionValues
{
public:
	OptionValues();
	virtual ~OptionValues();

	virtual std::string getOptionValue( std::string optionName );

	virtual bool containsOptionValue( std::string optionName );

	inline int size() { return values.size(); }
protected:
	virtual void setOptionValue( std::string optionName, std::string optionValue );

	friend class OptionSet;

private:
	std::map<std::string, std::string> values;
};

};

#endif /*_OPTIONVALUES_H_*/
