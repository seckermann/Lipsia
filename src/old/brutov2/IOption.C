/* global includes */
#include <string>
#include <map>
#include <list>
#include <iterator>
#include <algorithm>


/* local includes */
#include "IOption.h"
#include "IValidator.h"
#include "RequiredValidator.h"
#include "Logger.h"


using namespace std;
using namespace logger;

namespace option
{

IOption::IOption(string _name, string _shortName, string _help, string _shortHelp) {
	name      = _name;
	shortName = _shortName;
	help      = _help;
	shortHelp = _shortHelp;
	
	// REQUIRED, DEFAULT and HIDDEN attributes are always supported
	addSupportedAttribute("REQUIRED");
	addSupportedAttribute("DEFAULT");
	addSupportedAttribute("HIDDEN");
}

IOption::~IOption() {
}

void IOption::setAttribute(string key, string value) {
	attributes.insert(attributes.end(), make_pair(key, value));
}

void IOption::setAttributes(map<string, string> _attributes) {
	attributes.insert(_attributes.begin(), _attributes.end());
}

void IOption::addValidator(IValidator* _validator) {
	validators.push_back(_validator);
}

void IOption::addSupportedAttribute(string attributeName) {
	supportedAttributes.push_back(attributeName);
}

bool IOption::validate(string value) {
	bool _isValid = true; // think positive ;-)
	
	Logger::getInstance().debug("validating option " + getName());
	// let all validators have a look at the value
	list<IValidator*>::iterator vIterator = validators.begin();
//	list<IValidator>::iterator vIterator = validators.begin();
	while(vIterator != validators.end()) {
		if(!(*vIterator)->validate(value)) {
			_isValid = false;
		}
		++vIterator;
	}
	
	return _isValid;
}
	
bool IOption::checkAttributeNames() {
	bool _result = true;
	
	// all given attributes must be listed in the supportedAttributes list
	map<string, string>::iterator aIterator = attributes.begin();
	while(aIterator != attributes.end()) {
		if(count(supportedAttributes.begin(), supportedAttributes.end(), aIterator->first) == 0) {
			_result = false;
			Logger::getInstance().error("option " + name + " doesn't support the '" + aIterator->first + "' attribute"); 
		}
		++aIterator;
	}
	
	// The next one is a wee bit dodgy - but worked pretty well in the java implementation.
	// A valid option needs to either have a 'REQUIRED' or a 'DEFAULT' attribute (just one
	// of them - having both is considered an error)
	if(hasAttribute("DEFAULT") == hasAttribute("REQUIRED")) {
		_result = false;
		Logger::getInstance().error("option " + name + " can't have the 'REQUIRED' _AND_ the 'DEFAULT' attribute set"); 
	}
	
	return _result;
}

bool IOption::init() {
	if(!checkAttributeNames()) {
		return false;
	}
	
	if(attributes.find("REQUIRED") != attributes.end()) {
		addValidator(new RequiredValidator());
	}
	
	return true;
}


};
