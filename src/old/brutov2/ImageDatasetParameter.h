#ifndef _IMAGEDATASETPARAMETER_H_
#define _IMAGEDATASETPARAMETER_H_


/* global includes */
#include <string>
#include <vector>

/* local includes */


using namespace std;


namespace converter
{

class ImageDatasetParameter
{
public:
	ImageDatasetParameter();
	ImageDatasetParameter( const string &name, const string &value );
	ImageDatasetParameter( const string &name, const vector<string>& value );
	virtual ~ImageDatasetParameter();

	virtual string getName();

	virtual string getValue();

	virtual string getValue( int index );

	virtual int getValueNumber();

private:
	string         name;
	vector<string> value;
};

};

#endif /*_IMAGEDATASETPARAMETER_H_*/
