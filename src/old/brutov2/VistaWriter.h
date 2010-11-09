#ifndef _VISTAWRITER_H_
#define _VISTAWRITER_H_

/* global includes */
#include <vector>
#include "viaio/VImage.h"

/* local includes */
#include "IDatasetWriter.h"
#include "IImageDataset.h"
#include "ConverterConfiguration.h"

namespace vista
{

class VistaWriter : public converter::IDatasetWriter
{
public:
	VistaWriter(converter::ConverterConfiguration* configuration);
	virtual ~VistaWriter();
	virtual void write(std::vector<converter::IImageDataset*> datasets);

private:
	virtual VImage scaleSliceIntensity(VImage image, double black, double white, int fold = 0);
};

};

#endif /*_VISTAWRITER_H_*/
