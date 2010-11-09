/* global includes */
#include <iostream>

/* local includes */
#include "EmptyConverter.h"


namespace converter
{

EmptyConverter::EmptyConverter()
{
}

EmptyConverter::~EmptyConverter()
{
}

void EmptyConverter::convert() {
	std::cout << " - EmptyConverter.convert()" << std::endl;
}

};
