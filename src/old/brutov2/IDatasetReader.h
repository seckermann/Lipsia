#ifndef _IDATASETREADER_H_
#define _IDATASETREADER_H_

/* global includes */

/* local includes */
#include "IImageDataset.h"

namespace converter
{

class IDatasetReader
{
public:
	virtual IImageDataset *read() = 0;
};

};

#endif /*_IDATASETREADER_H_*/
