
/* global includes */
#include <iostream>
#include <fstream>
#include <ios>
#include <string>
#include <iterator>

/* local includes */
#include "JCampDXParser.h"
#include "JCampDXUtilities.h"
#include "Parameter.h"



using namespace std;


namespace jcampdx
{

JCampDXParser::JCampDXParser()
{
}

JCampDXParser::~JCampDXParser()
{
}

int JCampDXParser::parse(ifstream& jcampdxStream, map<string, Parameter>* parameterTable) {
	// see if the stream is open - if not bail out
	if(!jcampdxStream.is_open()) {
		return -1;
	}
	
	// clear the parameter list (we don't support appending parameters
	// to existing lists yet - don't even know if it would make any sense
	// at all)
	parameterTable->clear();
	
	// reset the stream to the very beginning (just in case someone
	// played around with it already)
	jcampdxStream.seekg(0, ios::beg);
	
	// now read the parameter file and extract all parameters
	string lineString;
	string parameterName;
	string parameterValue;
	while(!getline(jcampdxStream, lineString).eof()) {
		// see if we've got a new parameter
		// no idea (yet) why we can't just look for "##$" ??? =8-o
		// if(lineString.find_first_of("##$")) {
		if(lineString.find_first_of("##") == 0 && lineString.find_first_of("$") == 2) {
			// we found a new parameter so let's extract its name
			// NOTE: the name starts at position 3 (0-based) and its length is
			//       the position of "=" (0-based) minus 3 (that's where the 3 comes from)
			parameterName = lineString.substr(3, lineString.find_first_of("=") - 3);
			// reset the value string for our newly found parameter
			parameterValue = string();
			// let's see if we've got an array
			// NOTE 1: this is not ideal (we could cut-off the name and all the
			//         the other clutter and just work with the rest of the line)
			//         but for now it works just fine
			// NOTE 2: the position of the first "(" (in case of an array) is
			//         parameterName length + 3 ("##$") + 1 ("=")
			//         hence the '+ 4' in the next line (... magic ;-)
			if(lineString.find_first_of("(") == parameterName.size() + 4) {
				// "single line arrays" (that's what I call them) start immediately
				// after the left parenthesis (NO space) so we can use this to
				// distinguish between single and multi line arrays
				//
				// NOTE: we call them 'single line' but they can actually have more than one line
				// in the parameter file :-(
				if(lineString[parameterName.size() + 5] != ' ') {
					ifstream::pos_type _oldPosition = jcampdxStream.tellg();
					parameterValue += lineString.substr(parameterName.size() + 5, lineString.length() - 5);
					while(!getline(jcampdxStream, lineString).eof()) {
						// test for next parameter and end marker
						if((lineString.find_first_of("##") == 0 && lineString.find_first_of("$")    == 2) ||
						   (lineString.find_first_of("##") == 0 && lineString.find_first_of("END=") == 2) ||
						   (lineString.find_first_of("$")  == 0 && lineString.find_first_of("$", 1) == 1)) {
						   	// seek back to the end of the last line and bail out
						   	jcampdxStream.seekg(_oldPosition);
						   	break;
						}
						// we've got more 'value material' - append the line to our parameterValue string
						parameterValue += lineString;
						// store the position
						_oldPosition = jcampdxStream.tellg();
						// ... and on we go ...
					}
					vector<string> valueVector = JCampDXUtilities::tokenizeString(parameterValue, string(", "));
					parameterTable->insert(parameterTable->end(), make_pair(parameterName, Parameter(parameterName, valueVector)));
				} else {
					// multi line array
					// we need the old stream position
					ifstream::pos_type _oldPosition = jcampdxStream.tellg();
					// read all lines until we hit either the next parameter or the end marker "##END="
					// or a comment (starts with "$$")
					while(!getline(jcampdxStream, lineString).eof()) {
						// test for next parameter and end marker
						if((lineString.find_first_of("##") == 0 && lineString.find_first_of("$")    == 2) ||
						   (lineString.find_first_of("##") == 0 && lineString.find_first_of("END=") == 2) ||
						   (lineString.find_first_of("$")  == 0 && lineString.find_first_of("$", 1) == 1)) {
						   	// seek back to the end of the last line and bail out
						   	jcampdxStream.seekg(_oldPosition);
						   	break;
						}
						// we've got more 'value material' - append the line to our parameterValue string
						parameterValue += lineString;
						// store the position
						_oldPosition = jcampdxStream.tellg();
						// ... and on we go ...
					}
					// let's see what kind of array we deal with
					string::size_type _firstNonWhite = parameterValue.find_first_not_of(" ");
					// char arrays look like this: <abcdef>
					// we treat them as 'normal' strings without the brackets
					if(parameterValue[_firstNonWhite] == '<') {
						parameterTable->insert(parameterTable->end(), make_pair(parameterName, Parameter(parameterName, parameterValue.substr(_firstNonWhite + 1, parameterValue.find_last_of(">") - _firstNonWhite - 1))));
					} else {
						// this is a 'proper' array
						// break-up the value string with our tokenizer
						vector<string> valueVector = JCampDXUtilities::tokenizeString(parameterValue, string(" "));
						// and use the name and the value vector to create a new parameter which
						// gets appended to the parameter list
						parameterTable->insert(parameterTable->end(), make_pair(parameterName, Parameter(parameterName, valueVector)));
					}
				}
			} else {
				// this is just an 'ordinary' parameter - no big deal
				string _tmpValueString = lineString.substr(parameterName.size() + 4, lineString.size() - parameterName.size() - 4);
				// trim it
				// NOTE: we should consider writing some utility method/class for this sort of stuff
				string::size_type _firstNonWhite = _tmpValueString.find_first_not_of(" ");
				parameterValue = _tmpValueString.substr(_firstNonWhite, _tmpValueString.find_last_not_of(" ") - _firstNonWhite + 1);
				// now put it into the parameter list
				parameterTable->insert(parameterTable->end(), make_pair(parameterName, Parameter(parameterName, parameterValue)));
			}
		}
	}
	
	return 0;
}

};
