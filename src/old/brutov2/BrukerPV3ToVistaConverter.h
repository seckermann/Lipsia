#ifndef _BRUKERPV3TOVISTACONVERTER_H_
#define _BRUKERPV3TOVISTACONVERTER_H_

/* global includes */


/* local includes */
#include "Converter.h"
#include "ConverterConfiguration.h"



namespace converter
{

class BrukerPV3ToVistaConverter : public Converter
{

public:
	BrukerPV3ToVistaConverter( ConverterConfiguration *configuration );
	virtual ~BrukerPV3ToVistaConverter();

	virtual void convert();

private:
	ConverterConfiguration *configuration;
};

};

#endif /*_BRUKERPV3TOVISTACONVERTER_H_*/
