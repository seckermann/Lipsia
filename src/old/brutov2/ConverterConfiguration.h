#ifndef _CONVERTERCONFIGURATION_H_
#define _CONVERTERCONFIGURATION_H_


/* global includes */
#include <string>
#include <map>

/* local includes */


typedef std::map<std::string, std::string> cMap;


namespace converter
{

class ConverterConfiguration
{
public:
	ConverterConfiguration();
	ConverterConfiguration(const std::map<std::string, std::string>& configuration);
	virtual ~ConverterConfiguration();

	virtual void append(const std::string& key, const std::string& value);
	virtual void append(const std::map<std::string, std::string>& configuration);

	virtual std::map<std::string, std::string>& getConfiguration();
	virtual bool getConfigurationValue(const std::string& key, std::string& value);

	// constants for the names of commonly used configuration parameters
	const static std::string SOURCE_FORMAT_KEY;
	const static std::string TARGET_FORMAT_KEY;
	const static std::string SOURCE_URL_KEY;
	const static std::string TARGET_URL_KEY;
	const static std::string APPLICATION_NAME_KEY;
	const static std::string APPLICATION_VERSION_KEY;
	const static std::string APPLICATION_COMMAND_LINE_OPTIONS_KEY;


	// constants for the data formats we deal with

	/* Bruker ParaVision 3 */
	const static std::string BRUKER_PV3_FORMAT;
	/* Bruker ParaVision 1.1.x */
	const static std::string BRUKER_PV11_FORMAT;
	/* Bruker ParaVision (general) NOTE: this one means we need to find out what exactly we deal with */
	const static std::string BRUKER_FORMAT;
	/* DICOM with Siemens specific stuff */
	const static std::string DICOM_SIEMENS_FORMAT;
	/* good old Vista */
	const static std::string VISTA_FORMAT;
	/* Nifti */
	const static std::string NIFTI_FORMAT;

private:
	/*
	 * A key-value map representing this ConverterConfiguration.
	 *
	 * This map contains all the information a specific Converter needs
	 * to finally convert data from one format into another one - meaning
	 * all the information coming from commandline options, configuration
	 * files or even GUI dialogs ends up right here and is used by the
	 * choosen Converter implementation (@see ConverterFactory) to do the
	 * actual job.
	 */
	cMap _configuration;


};


};

#endif /*_CONVERTERCONFIGURATION_H_*/
