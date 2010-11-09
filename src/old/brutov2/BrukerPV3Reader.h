#ifndef _BRUKERPV3READER_H_
#define _BRUKERPV3READER_H_


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


class BrukerPV3Reader : public converter::IDatasetReader
{
public:
	BrukerPV3Reader(converter::ConverterConfiguration* configuration);
	virtual ~BrukerPV3Reader();
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
	// 'method' parameter table
	parameterTable methodParameters;
	// 'acqp' parameter table
	parameterTable acqpParameters;
	// 'reco' parameter table
	parameterTable recoParameters;
	// 'd3proc' parameter table
	parameterTable d3procParameters;
	// 'imnd' parameter table
	parameterTable imndParameters;

	bool hasMethodFile;


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
	const static string SOURCE_FORMAT_FLOAT;
	
};

};

#endif /*_BRUKERPV3READER_H_*/
