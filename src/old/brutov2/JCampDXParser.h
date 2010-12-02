#ifndef _JCAMPDXPARSER_H_
#define _JCAMPDXPARSER_H_

/* global includes */
#include <fstream>
#include <map>

/* local includes */
#include "Parameter.h"


namespace jcampdx
{

class JCampDXParser
{
public:
	JCampDXParser();
	virtual ~JCampDXParser();

	virtual int parse( std::ifstream &jcampdxStream, std::map<string, Parameter>* parameterTable );

	const std::string getDescription() { return "JCampDXParser version 0.1"; }
};

};

#endif /*_JCAMPDXPARSER_H_*/
