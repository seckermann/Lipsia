#ifndef _BRUKERPV3TONIFTICONVERTER_H_
#define _BRUKERPV3TONIFTICONVERTER_H_

/* global includes */


/* local includes */
#include "Converter.h"
#include "ConverterConfiguration.h"



namespace converter
{

class BrukerPV3ToNiftiConverter : public Converter
{

public:
	BrukerPV3ToNiftiConverter( ConverterConfiguration *configuration );
	virtual ~BrukerPV3ToNiftiConverter();

	virtual void convert();

private:
	ConverterConfiguration *configuration;
};

};

#endif /*_BRUKERPV3TONIFTICONVERTER_H_*/
