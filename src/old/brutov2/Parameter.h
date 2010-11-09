#ifndef _PARAMETER_H_
#define _PARAMETER_H_


/* global includes */
#include <string>
#include <vector>

/* local includes */


using namespace std;


namespace jcampdx
{

class Parameter
{
public:
	Parameter();
	Parameter(const string& name, const string& value);
	Parameter(const string& name, const vector<string>& value);
	virtual ~Parameter();
	
	virtual string getName();
	
	virtual int    getValueNumber();
	
 	virtual string getValue();
	
	virtual string getValue(int index);
	
private:
	string         name;
	vector<string> value;
};

};

#endif /*_PARAMETER_H_*/
