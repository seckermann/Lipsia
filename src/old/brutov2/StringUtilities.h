#ifndef _STRINGUTILITIES_H_
#define _STRINGUTILITIES_H_

/* global includes */
#include <string>
#include <vector>

/* local includes */


using namespace std;

namespace strutils
{

class StringUtilities
{

public:
	static vector<string> tokenizeString( const string &input, const string &delimiters = "," ) {
		// the resulting vector of tokens
		vector<string> tokens;
		// find the first non-delimiter character
		string::size_type lastPos = input.find_first_not_of( delimiters, 0 );
		// find next delimiter character
		string::size_type pos = input.find_first_of( delimiters, lastPos );

		while ( string::npos != pos || string::npos != lastPos ) {
			// token found - add it to the vector.
			tokens.push_back( input.substr( lastPos, pos - lastPos ) );
			// skip delimiter characters
			lastPos = input.find_first_not_of( delimiters, pos );
			// next delimiter character
			pos = input.find_first_of( delimiters, lastPos );
		}

		return tokens;
	}


};


};

#endif /*_STRINGUTILITIES_H_*/
