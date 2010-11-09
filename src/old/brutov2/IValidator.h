#ifndef _IVALIDATOR_H_
#define _IVALIDATOR_H_

/* global includes */
#include <string>
#include <map>


/* local includes */


namespace option
{

class IValidator
{
public:
	
	virtual ~IValidator() {};
	virtual bool validate(std::string value) = 0;
	
	inline void setAttribute(std::string name, std::string value) {
		attributes.insert(attributes.end(), make_pair(name, value));
	}

	inline std::string getAttribute(std::string name) {
		return attributes[name];
	}

	inline bool hasAttribute(std::string name) {
		return (attributes.find(name) != attributes.end());
	}

private:
	std::map<std::string, std::string> attributes;
};

};

#endif /*_IVALIDATOR_H_*/
