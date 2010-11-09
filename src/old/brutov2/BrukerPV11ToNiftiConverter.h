#ifndef _BRUKERPV11TONIFTICONVERTER_H_
#define _BRUKERPV11TONIFTICONVERTER_H_

/* global includes */


/* local includes */
#include "Converter.h"
#include "ConverterConfiguration.h"



namespace converter
{

class BrukerPV11ToNiftiConverter : public Converter {

public:
	BrukerPV11ToNiftiConverter(ConverterConfiguration* configuration);
	virtual ~BrukerPV11ToNiftiConverter();

	virtual void convert();

private:
	ConverterConfiguration* configuration;

};

};

#endif /*_BRUKERPV11TONIFTICONVERTER_H_*/
