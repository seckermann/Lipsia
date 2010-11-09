#include "Parameter.h"

#include <iostream>

using namespace std;

namespace jcampdx
{

Parameter::Parameter()
{
	name = string("");
	value = vector<string>(1, "");
}

Parameter::Parameter(const string& n, const string& v)
{
	name = string(n);
	value = vector<string>(1, v);
}

Parameter::Parameter(const string& n, const vector<string>& v)
{
	name = string(n);
	value = vector<string>(v.begin(), v.end());
}

Parameter::~Parameter()
{
}

string Parameter::getName() 
{
	return name;
}
	
string Parameter::getValue()
{
	return value[0];
}
	
int Parameter::getValueNumber()
{
	return value.size();
}
	
string Parameter::getValue(int index)
{
	// we need to check the size of the array
	return value[index];
}

};
