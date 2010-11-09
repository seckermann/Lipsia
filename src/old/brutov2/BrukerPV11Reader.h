#ifndef _BRUKERPV11READER_H_
#define _BRUKERPV11READER_H_

/* global includes */
#include <iostream>
#include <string>
#include <map>

/* local includes */
#include "IDatasetReader.h"
#include "IImageDataset.h"
#include "ImageDatasetParameter.h"
#include "ConverterConfiguration.h"
#include "JCampDXParser.h"


using namespace converter;
using namespace jcampdx;


namespace bruker
{

typedef std::map<std::string, Parameter> parameterTable;


class BrukerPV11Reader : public converter::IDatasetReader
{
public:
	BrukerPV11Reader(converter::ConverterConfiguration* configuration);
	virtual ~BrukerPV11Reader();
	virtual converter::IImageDataset* read();
	
	// our converter configuration
	ConverterConfiguration* configuration;
	// the root directory of the dataset this reader deals with
	string rootDirectory;
	// the experiment number as string
	string experimentNumber;
	// the reco number as string
	string recoNumber;
	// the actual data file stream
	string recoDataFileName;
	// the actual data file stream
	ifstream* recoDataFileStream;
	// 'subject' parameter table
	parameterTable subjectParameters;
	// 'acqp' parameter table
	parameterTable acqpParameters;
	// 'imnd' parameter table
	parameterTable imndParameters;
	// 'gui' parameter table
	parameterTable guiParameters;
	// 'reco' parameter table
	parameterTable recoParameters;
	// 'd3proc' parameter table
	parameterTable d3procParameters;

	// some byte format related constants
	// NOTE: with 'int' we mean 4 bytes and with 'long' we mean 8 bytes as
	// known since the stone ages
	const static string SOURCE_FORMAT_BIT;
	const static string SOURCE_FORMAT_SIGNED_BYTE;
	const static string SOURCE_FORMAT_UNSIGNED_BYTE;
	const static string SOURCE_FORMAT_SIGNED_SHORT;
	const static string SOURCE_FORMAT_UNSIGNED_SHORT;
	const static string SOURCE_FORMAT_SIGNED_INT;
	const static string SOURCE_FORMAT_UNSIGNED_INT;
	const static string SOURCE_FORMAT_SIGNED_LONG;
	const static string SOURCE_FORMAT_UNSIGNED_LONG;

};

};
#endif /*_BRUKERPV11READER_H_*/
