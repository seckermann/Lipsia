#ifndef _IDATASETWRITER_H_
#define _IDATASETWRITER_H_

/* global includes */
#include <vector>

/* local includes */
#include "IImageDataset.h"


namespace converter
{

class IDatasetWriter
{
public:
	virtual void write(std::vector<IImageDataset*> datasets) = 0;
};

};

#endif /*_IDATASETWRITER_H_*/
