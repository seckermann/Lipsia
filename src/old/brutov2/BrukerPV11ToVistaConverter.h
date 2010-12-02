#ifndef _BRUKERPV11TOVISTACONVERTER_H_
#define _BRUKERPV11TOVISTACONVERTER_H_

/* global includes */


/* local includes */
#include "Converter.h"
#include "ConverterConfiguration.h"



namespace converter
{

class BrukerPV11ToVistaConverter : public Converter
{

public:
	BrukerPV11ToVistaConverter( ConverterConfiguration *configuration );
	virtual ~BrukerPV11ToVistaConverter();

	virtual void convert();

private:
	ConverterConfiguration *configuration;

};

};

#endif /*_BRUKERPV11TOVISTACONVERTER_H_*/
