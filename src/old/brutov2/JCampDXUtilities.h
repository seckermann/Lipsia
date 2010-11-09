#ifndef _JCAMPDXUTILITIES_H_
#define _JCAMPDXUTILITIES_H_

/* global includes */
#include <string>
#include <vector>

/* local includes */



using namespace std;

namespace jcampdx
{

/*
 * A class containing various (static) utility methods for the
 * JCampDX parser.
 */
class JCampDXUtilities
{
public:
	/*
	 * A method to 'tokenize' an input string using the given delimiter characters.
	 * 
	 * @param input        the input string to tokenize
	 * @param delimiters   a string containing all delimiter characters
	 * 
	 * @return   a vector of strings containing the 'tokens'.
	 */
	static vector<string> tokenizeString(const string& input, const string& delimiters = " ") {
		// the resulting vector of tokens
		vector<string> tokens;
	    // find the first non-delimiter character
    	string::size_type lastPos = input.find_first_not_of(delimiters, 0);
    	// find next delimiter character
    	string::size_type pos = input.find_first_of(delimiters, lastPos);

	    while (string::npos != pos || string::npos != lastPos) {
	        // token found - add it to the vector.
    	    tokens.push_back(input.substr(lastPos, pos - lastPos));
        	// skip delimiter characters
	        lastPos = input.find_first_not_of(delimiters, pos);
    	    // next delimiter character
        	pos = input.find_first_of(delimiters, lastPos);
	    }
	    
	    return tokens;
	}
	
	/*
	 * A method to compose a string (i.e. for printing) from the given vector and array
	 * dimensions. Given the fact that vector is one-dimensional we need the intended
	 * array dimensions to be able to deal with multi-dimensional arrays contained in the
	 * input vector.
	 * 
	 * NOTE: this public method is just a wrapper for the recursive private createString()
	 * method which does the actual work.
	 * 
	 * @param array             a vector containing the elements
	 * @param arrayDimensions   a vector containing the dimensions of the 'array'
	 * @param delimiter         the delimiter to use in the resulting output string (defaults to ", ")
	 * 
	 * @return   the resulting output string reflecting the given array dimensions
	 */
	static string toString(const vector<string>& array, const vector<int>& arrayDimensions, const string& delimiter = ", ") {
		// a copy of our input vector
		vector<string> _array(array.begin(), array.end());
		// a copy of our dimension vector
		vector<int> _dimensions(arrayDimensions.begin(), arrayDimensions.end());
		
		string _outputString = string("");
		createString(_outputString, _array, _dimensions, delimiter);
		return _outputString;
	}
	
private:

	/*
	 * A recursive method to create a string representation of a vector of input strings according to the
	 * given dimensions and delimiter.
	 * 
	 * NOTE: although it's not a very complicated species, don't even try to understand this - understanding
	 * someone else's recursive methods is always a bit of bugger ;-)
	 */
	static void createString(string& output, vector<string>& array, vector<int>& dimensions, const string& delimiter) {
		if(dimensions.size() == 1) {
			output += string("[");
			for(int count = 0; count < dimensions[0]; count++) {
				output += array.front();
				array.erase(array.begin());
				if(count < dimensions[0] - 1) {
					output += delimiter;
				}
			}
			output += string("]");
		} else {
			vector<int> nextDimensions(dimensions.begin() + 1, dimensions.end());
			output += string("[");
			for(int count = 0; count < dimensions[0]; count++) {
				createString(output, array, nextDimensions, delimiter);
				if(count < dimensions[0] - 1) {
					output += delimiter;
				}
			}
			output += string("]");
		}
	}
};

};

#endif /*_JCAMPDXUTILITIES_H_*/
