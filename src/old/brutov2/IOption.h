#ifndef _IOPTION_H_
#define _IOPTION_H_


/* global includes */
#include <string>
#include <list>
#include <map>

/* local includes */
#include "IValidator.h"

namespace option
{

class IOption
{
public:
	IOption( std::string name, std::string shortName, std::string help, std::string shortHelp );
	virtual ~IOption();

	virtual void setAttribute( std::string key, std::string value );
	virtual void setAttributes( std::map<std::string, std::string> attributes );
	virtual void addValidator( IValidator *validator );

	inline std::string getName()      { return name; }
	inline std::string getShortName() { return shortName; }
	inline std::string getHelp()      { return help; }
	inline std::string getShortHelp() { return shortHelp; }

	inline std::string getAttribute( std::string name ) { return attributes[name]; }
	inline bool hasAttribute( std::string name ) { return ( attributes.find( name ) != attributes.end() ); }

	virtual bool init();
protected:

	virtual void addSupportedAttribute( std::string attributeName );
	// TODO: should(??) have either a boolean parameter saying whether to report
	// an error to something like cerr or a string& parameter to place an
	// error message there or an ostream* parameter to write the message to
	// NOTE: we had the boolean option implemented in the java version and it
	// worked pretty well - but there we mainly had to fight against UI related odds.
	// NOTE2: we should really think about exceptions - because this case seems to
	// ask for it
	virtual bool validate( std::string value );


	virtual bool checkAttributeNames();
private:
	std::string name;
	std::string shortName;
	std::string help;
	std::string shortHelp;

	std::list<std::string>             supportedAttributes;
	std::map<std::string, std::string> attributes;
	std::list<IValidator *>             validators;

	friend class OptionSet;
	friend class StringListOption;
};

};

#endif /*_IOPTION_H_*/
