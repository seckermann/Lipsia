#ifndef _EMPTYCONVERTER_H_
#define _EMPTYCONVERTER_H_


/* global includes */


/* local includes */
#include "Converter.h"


namespace converter
{

class EmptyConverter : public Converter
{
public:
	EmptyConverter();
	virtual ~EmptyConverter();

	virtual void convert();
};

};

#endif /*_EMPTYCONVERTER_H_*/
