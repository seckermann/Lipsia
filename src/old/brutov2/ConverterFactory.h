#ifndef _CONVERTERFACTORY_H_
#define _CONVERTERFACTORY_H_

/* global includes */

/* local includes */
#include "Converter.h"
#include "ConverterConfiguration.h"


namespace converter
{

/*
 * A factory [GOF 95] for Converter implementations. Depending on the given ConverterConfiguration
 * this factory returns a suitable Converter implementation. It's implemented as a singleton [GOF 95]
 * because we really only need one factory instance.
 * 
 * For now we won't bother doing any 'magical' things like the detection of the source data format and so on.
 * We simply expect the given ConverterConfiguration to contain certain parameters. At the moment these are:
 * 
 * SourceFormat:   the format of the source data (e.g 'PV3' (this is Bruker ParaVision 3), 'DICOM' (specific flavours), 'VISTA'...)
 * SourceURL:      where is the source data (supp. URL types: file, http, ftp - at the moment)
 * TargetFormat:   the format of the target data (e.g 'PV3', 'DICOM' (specific flavours), 'VISTA', or whatever we decide to implement)
 * TargetURL:      where to put the target data (supp. URL types: file, http, ftp - at the moment)
 */
class ConverterFactory
{
	
	
public:

	inline static ConverterFactory& getInstance() {
		static ConverterFactory instance;
		return instance;
	}
	
	virtual Converter* getConverter(ConverterConfiguration* configuration);


private : 

    inline explicit ConverterFactory() {}
    virtual ~ConverterFactory() {}
    inline explicit ConverterFactory(ConverterFactory const&) {}
    inline ConverterFactory& operator=(ConverterFactory const&) { return *this; }


};

};
#endif /*_CONVERTERFACTORY_H_*/
