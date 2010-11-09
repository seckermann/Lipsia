#ifndef _NIFTIWRITER_H_
#define _NIFTIWRITER_H_

/* global includes */
#include <vector>
#include <string>
#include <sstream>
#include <nifti/nifti1.h>

/* local includes */
#include "IDatasetWriter.h"
#include "IImageDataset.h"
#include "ConverterConfiguration.h"

namespace nifti
{

class NiftiWriter : public converter::IDatasetWriter
{
public:
	NiftiWriter(converter::ConverterConfiguration* configuration);
	virtual ~NiftiWriter();
	virtual void write(std::vector<converter::IImageDataset*> datasets);

private:
	virtual void writeOrientation(converter::IImageDataset* dataset, nifti_1_header& nHeader);

	virtual vector<float> matrixProduct(vector<float> matrixA, vector<float> matrixB);

	template<class T> std::string toString(T number) {
		std::ostringstream os;
		os << number;
		return os.str();
	}
};

};

#endif /*_NIFTIWRITER_H_*/
