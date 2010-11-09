#include "ImageDatasetParameter.h"


using namespace std;


namespace converter
{


ImageDatasetParameter::ImageDatasetParameter()
{
	name = string("");
	value = vector<string>(1, "");
}

ImageDatasetParameter::ImageDatasetParameter(const string& n, const string& v)
{
	name = string(n);
	value = vector<string>(1, v);
}

ImageDatasetParameter::ImageDatasetParameter(const string& n, const vector<string>& v)
{
	name = string(n);
	value = vector<string>(v.begin(), v.end());
}

ImageDatasetParameter::~ImageDatasetParameter()
{
}

string ImageDatasetParameter::getName() 
{
	return name;
}
	
string ImageDatasetParameter::getValue()
{
	return value[0];
}
	
string ImageDatasetParameter::getValue(int index)
{
	// we need to check the size of the array
	return value[index];
}

int ImageDatasetParameter::getValueNumber() {
	return value.size();
}

};
