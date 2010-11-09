/* global includes */
#include <string>
#include <vector>
#include <list>

/* local includes */
#include "OptionSet.h"
#include "StringUtilities.h"
#include "Logger.h"

using namespace std;
using namespace strutils;
using namespace logger;


namespace option
{

typedef list<IOption*>::iterator optionsIterator;


OptionSet::OptionSet() {
}

OptionSet::~OptionSet() {
}

bool OptionSet::addOption(IOption* option) {
	if(option->init()) {
		options.push_back(option);
		return true;
	} else {
		Logger::getInstance().error("init() for option " + option->getName() + " returned false");
		return false;
	}
}

bool OptionSet::parse(vector<string> optionVector, OptionValues& values) {
	bool _result = true;

	/*
	 * TODO: this whole parsing stuff needs to be more robust - we'll do this later on
	 */

	vector<string>::iterator ovIterator = optionVector.begin();
	while(ovIterator != optionVector.end()) {
		if((*ovIterator).compare(0, 2, "--") == 0) {
			// got a 'long option'
			IOption* _op = findByName((*ovIterator).substr(2, (*ovIterator).size() - 2));
			// state a warning if the option is not registered
			if(_op == NULL) {
				Logger::getInstance().warning("unknown option: " + (*ovIterator));
				break;
			}
			Logger::getInstance().debug("OptionSet found option '" + (*ovIterator) + "'");
			// get the option value
			ovIterator++;
			if(ovIterator == optionVector.end() || (*ovIterator).compare(0, 1, "-") == 0) { // BooleanOption is special
				values.setOptionValue(_op->getName(), "yes");
				ovIterator--;
			} else {
				Logger::getInstance().debug("OptionSet found option value '" + (*ovIterator) + "'");
				// validate it
				if(!_op->validate((*ovIterator))) {
					_result = false;
				}
				// add it to our OptionValues table
				values.setOptionValue(_op->getName(), (*ovIterator));
			}
		} else if((*ovIterator).compare(0, 1, "-") == 0) {
			// got a 'short option'
			IOption* _op = findByShortName((*ovIterator).substr(1, (*ovIterator).size() - 1));
			// state a warning if the option is not registered
			if(_op == NULL) {
				Logger::getInstance().warning("unknown option: " + (*ovIterator));
				break;
			}
			Logger::getInstance().debug("OptionSet found option '" + (*ovIterator) + "'");
			// get the option value
			ovIterator++;
			if(ovIterator == optionVector.end() || (*ovIterator).compare(0, 1, "-") == 0) { // BooleanOption is special
				values.setOptionValue(_op->getName(), "yes");
				ovIterator--;
			} else {
				Logger::getInstance().debug("OptionSet found option value '" + (*ovIterator) + "'");
				// validate it
				if(!_op->validate((*ovIterator))) {
					_result = false;
				}
				// add it to our OptionValues table
				values.setOptionValue(_op->getName(), (*ovIterator));
			}
		}
		++ovIterator;
	}
	
	// set default values for options which were not given but have a 'DEFAULT' attribute
	// and validate them (in case the given 'DEFAULT' attribute is wrong - which it shouldn't) 
	//
	// TODO: this whole thing should actually be the other way 'round - first add the missing
	//       options (with DEFAULT attribute) to the optionVector and then validate all of it
	//       at once
	optionsIterator _odIterator = options.begin();
	while(_odIterator != options.end()) {
		if((*_odIterator)->hasAttribute("DEFAULT") && !values.containsOptionValue((*_odIterator)->getName())) {
			string _value = (*_odIterator)->attributes["DEFAULT"];
			if(!(*_odIterator)->validate(_value)) {
				Logger::getInstance().error("Option '" + (*_odIterator)->getName() + "' has an invalid default value: " + _value);
				_result = false;
			}
			values.setOptionValue((*_odIterator)->getName(), _value);
		}
		++_odIterator;
	}

	// check if we've got all the required options
	// NOTE: this needs more thought because it renders our
	// RequiredValidator obsolete - it works but isn't very pretty
	optionsIterator _orIterator = options.begin();
	while(_orIterator != options.end()) {
		if((*_orIterator)->hasAttribute("REQUIRED") && !values.containsOptionValue((*_orIterator)->getName())) {
			Logger::getInstance().error("Option '" + (*_orIterator)->getName() + "' is required");
			_result = false;
		}
		++_orIterator;
	}
	

	return _result;
}


IOption* OptionSet::findByName(string name) {
	optionsIterator oIterator = options.begin();
	while(oIterator != options.end()) {
		if((*oIterator)->getName().compare(name) == 0) {
			return (*oIterator);
		}
		++oIterator;
	}
	return NULL;
}

IOption* OptionSet::findByShortName(string shortName) {
	optionsIterator oIterator = options.begin();
	while(oIterator != options.end()) {
		if((*oIterator)->getShortName().compare(shortName) == 0) {
			return (*oIterator);
		}
		++oIterator;
	}
	return NULL;
}

void OptionSet::printHelp() {
	Logger& _l = Logger::getInstance();
	Logger::LogLevel _oll = _l.setLogLevel(Logger::INFO);
	bool _osdt = _l.setShowDateTime(false);
	bool _osll = _l.setShowLevel(false);

	_l.info("");
	_l.info("Options:");
	_l.info("");
	optionsIterator oIterator = options.begin();
	while(oIterator != options.end()) {
		if(!(*oIterator)->hasAttribute("HIDDEN")) {
			string name      = (*oIterator)->getName();
			string shortName = (*oIterator)->getShortName();
			string help      = (*oIterator)->getHelp();
		
			_l.info("--" + name);
			_l.info(" -" + shortName);
			_l.info("\t" + help);
			_l.info("");
		}
		++oIterator;
	}
	
	_l.setLogLevel(_oll);
	_l.setShowDateTime(_osdt);
	_l.setShowLevel(_osll);
}

void OptionSet::printExtendedHelp() {
	Logger& _l = Logger::getInstance();
	Logger::LogLevel _oll = _l.setLogLevel(Logger::INFO);
	bool _osdt = _l.setShowDateTime(false);
	bool _osll = _l.setShowLevel(false);

	_l.info("");
	_l.info("Extended Options:");
	_l.info("");
	optionsIterator oIterator = options.begin();
	while(oIterator != options.end()) {
		string name      = (*oIterator)->getName();
		string shortName = (*oIterator)->getShortName();
		string help      = (*oIterator)->getHelp();
	
		_l.info("--" + name);
		_l.info(" -" + shortName);
		_l.info("\t" + help);
		_l.info("");

		++oIterator;
	}
	

	_l.setLogLevel(_oll);
	_l.setShowDateTime(_osdt);
	_l.setShowLevel(_osll);
}


};
