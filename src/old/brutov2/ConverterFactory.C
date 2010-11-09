/* global includes */
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

/* local includes */
#include "ConverterFactory.h"
#include "ConverterConfiguration.h"
#include "Converter.h"
#include "EmptyConverter.h"
#include "BrukerPV3ToVistaConverter.h"
#include "BrukerPV11ToVistaConverter.h"
#include "BrukerPV3ToNiftiConverter.h"
#include "BrukerPV11ToNiftiConverter.h"
#include "Logger.h"
#include "StringUtilities.h"


using namespace std;
using namespace logger;
using namespace strutils;


namespace converter
{


Converter* ConverterFactory::getConverter(ConverterConfiguration* configuration) {

	Converter* _conv(NULL);

	string _sourceFormat, _targetFormat;

	if(configuration->getConfigurationValue(ConverterConfiguration::SOURCE_FORMAT_KEY, _sourceFormat) &&
	   configuration->getConfigurationValue(ConverterConfiguration::TARGET_FORMAT_KEY, _targetFormat)) {

	   	Logger::getInstance().debug("[ConverterFactory] SourceFormat = " + _sourceFormat);
	   	Logger::getInstance().debug("[ConverterFactory] TargetFormat = " + _targetFormat);
	   	// see if we've got the right converter
	   	if(_sourceFormat == ConverterConfiguration::BRUKER_PV3_FORMAT &&
	   	   _targetFormat == ConverterConfiguration::VISTA_FORMAT) {
	   	   	// this is Bruker PV3 to Vista - the one we support at the moment
	   	   	// create the converter with the given configuration and return it
	   	   	_conv = new BrukerPV3ToVistaConverter(configuration);
	   	} else if(_sourceFormat == ConverterConfiguration::BRUKER_PV11_FORMAT &&
	   	          _targetFormat == ConverterConfiguration::VISTA_FORMAT) {
	   	    _conv = new BrukerPV11ToVistaConverter(configuration);
	   	} else if(_sourceFormat == ConverterConfiguration::BRUKER_FORMAT &&
	   			(_targetFormat == ConverterConfiguration::VISTA_FORMAT || _targetFormat == ConverterConfiguration::NIFTI_FORMAT)) {
			string experimentsToConvert;
			configuration->getConfigurationValue("bruker.experiments", experimentsToConvert);
			vector<string> experiments = StringUtilities::tokenizeString(experimentsToConvert, ",");
			string experimentNumber = StringUtilities::tokenizeString(experiments[0], ".")[0];

	   	    string rootDirectory;
			configuration->getConfigurationValue(ConverterConfiguration::SOURCE_URL_KEY, rootDirectory);
			rootDirectory.erase(0, rootDirectory.find_first_of(':') + 1);

			Logger::getInstance().debug("[ConverterFactory] source directory: " + rootDirectory);

			// test if experiment exists
			ifstream expDirStream((rootDirectory + "/" + experimentNumber).c_str());
			if(!expDirStream) {
				Logger::getInstance().error("[ConverterFactory] experiment directory does not exist: " + (rootDirectory + "/" + experimentNumber));
				return NULL;
			}


			ifstream methodStream((rootDirectory + "/" + experimentNumber + "/" + "method").c_str());
			ifstream acqpStream(  (rootDirectory + "/" + experimentNumber + "/" + "acqp").c_str()  );
			ifstream imndStream(  (rootDirectory + "/" + experimentNumber + "/" + "imnd").c_str()  );
			ifstream guiStream(   (rootDirectory + "/" + experimentNumber + "/" + "gui").c_str()   );
			ifstream pdataStream( (rootDirectory + "/" + experimentNumber + "/" + "pdata").c_str() );
			if(acqpStream && imndStream && guiStream && pdataStream) {
				if(_targetFormat == ConverterConfiguration::VISTA_FORMAT) {
					_conv = new BrukerPV11ToVistaConverter(configuration);
					Logger::getInstance().debug("[ConverterFactory] using BrukerPV11ToVistaConverter");
				}
				if(_targetFormat == ConverterConfiguration::NIFTI_FORMAT) {
					_conv = new BrukerPV11ToNiftiConverter(configuration);
					Logger::getInstance().debug("[ConverterFactory] using BrukerPV11ToNiftiConverter");
				}
			} else if(acqpStream && pdataStream) {
				if(_targetFormat == ConverterConfiguration::VISTA_FORMAT) {
					_conv = new BrukerPV3ToVistaConverter(configuration);
					Logger::getInstance().debug("[ConverterFactory] using BrukerPV3ToVistaConverter");
				}
				if(_targetFormat == ConverterConfiguration::NIFTI_FORMAT) {
					_conv = new BrukerPV3ToNiftiConverter(configuration);
					Logger::getInstance().debug("[ConverterFactory] using BrukerPV3ToNiftiConverter");
				}
			}
			if(expDirStream) { expDirStream.close(); }
			if(methodStream) { methodStream.close(); }
			if(acqpStream)   { acqpStream.close();   }
			if(imndStream)   { imndStream.close();   }
			if(guiStream)    { guiStream.close();    }
			if(pdataStream)  { pdataStream.close();  }
	   	}
	   	return _conv;
	} else {
		// In the future we will have routines to automagically detect the
		// formats (not a big deal with the source, but I'm not so sure about
		// the target format) .
		// For now, not having both, source and target format explicitly configured
		// is regarded an error.

		_conv = new EmptyConverter(); // just return the empty converter for the moment
		return _conv;
	}


	_conv = new EmptyConverter();
	return _conv;
}

};
