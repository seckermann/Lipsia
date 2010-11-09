/* local includes */
#include "ConverterConfiguration.h"

/* global includes */


using namespace std;

typedef std::map<std::string, std::string> cMap;

namespace converter
{



ConverterConfiguration::ConverterConfiguration()
{
	_configuration.clear();
}

ConverterConfiguration::ConverterConfiguration(const cMap& configuration)
{
	_configuration.insert(configuration.begin(), configuration.end());
}

ConverterConfiguration::~ConverterConfiguration()
{
}

void ConverterConfiguration::append(const string& key, const string& value)
{
	_configuration.insert(_configuration.end(), make_pair(key, value));
}

void ConverterConfiguration::append(const cMap& configuration)
{
	_configuration.insert(configuration.begin(), configuration.end());
}

cMap& ConverterConfiguration::getConfiguration() {
	return _configuration;
}

bool ConverterConfiguration::getConfigurationValue(const string& key, string& value) {
	if(_configuration.find(key) != _configuration.end()) {
		value = string(_configuration[key]);
		return true;
	} else {
		return false;
	}
}

// configuration key constants
const string ConverterConfiguration::SOURCE_FORMAT_KEY                    = "SourceFormat";
const string ConverterConfiguration::TARGET_FORMAT_KEY                    = "TargetFormat";
const string ConverterConfiguration::SOURCE_URL_KEY                       = "SourceURL";
const string ConverterConfiguration::TARGET_URL_KEY                       = "TargetURL";
const string ConverterConfiguration::APPLICATION_NAME_KEY                 = "ApplicationName";
const string ConverterConfiguration::APPLICATION_VERSION_KEY              = "ApplicationVersion";
const string ConverterConfiguration::APPLICATION_COMMAND_LINE_OPTIONS_KEY = "ApplicationCommandLineOptions";

// data format constants
const string ConverterConfiguration::BRUKER_PV3_FORMAT    = "BrukerPV3";
const string ConverterConfiguration::BRUKER_PV11_FORMAT   = "BrukerPV11";
const string ConverterConfiguration::BRUKER_FORMAT        = "Bruker";
const string ConverterConfiguration::DICOM_SIEMENS_FORMAT = "DicomSiemens";
const string ConverterConfiguration::VISTA_FORMAT         = "Vista";
const string ConverterConfiguration::NIFTI_FORMAT         = "Nifti";
};
