/* global includes */
#include <string>

/* local includes */
#include "OptionValues.h"


namespace option
{

OptionValues::OptionValues() {
}

OptionValues::~OptionValues() {
}

std::string OptionValues::getOptionValue(std::string optionName) {
	return values[optionName];
}

bool OptionValues::containsOptionValue(std::string optionName) {
	return (values.find(optionName) != values.end());
}
	
void OptionValues::setOptionValue(std::string optionName, std::string optionValue) {
	values.insert(values.end(), make_pair(optionName, optionValue));
}

};
