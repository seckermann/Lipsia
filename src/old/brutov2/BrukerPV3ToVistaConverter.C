
/* global includes */
#include <vector>
#include <string>
#include <sstream>

/* local includes */
#include "BrukerPV3ToVistaConverter.h"
#include "ConverterConfiguration.h"
#include "IDatasetReader.h"
#include "IDatasetWriter.h"
#include "IImageDataset.h"
#include "Logger.h"
#include "StringUtilities.h"
#include "BrukerPV3Reader.h"
#include "VistaWriter.h"


using namespace bruker;
using namespace vista;
using namespace logger;
using namespace strutils;
using namespace std;

namespace converter
{

BrukerPV3ToVistaConverter::BrukerPV3ToVistaConverter(ConverterConfiguration* _cc) {
	
	configuration = _cc;
}

BrukerPV3ToVistaConverter::~BrukerPV3ToVistaConverter() {
}

void BrukerPV3ToVistaConverter::convert() {
	Logger::getInstance().debug("[BrukerPV3ToVistaConverter] convert()");

	IDatasetReader* reader;
	IDatasetWriter* writer;

	vector<IImageDataset*> datasets;

	string experimentsToConvert;
	configuration->getConfigurationValue("bruker.experiments", experimentsToConvert);
	vector<string> experiments = StringUtilities::tokenizeString(experimentsToConvert, ",");

	// iterate over all requested experiments
	vector<string>::iterator experiment = experiments.begin();
	while(experiment != experiments.end()) {
		Logger::getInstance().debug("[BrukerPV3ToVistaConverter] analyzing experiment selection parameter: " + (*experiment));
		ConverterConfiguration* dscc = new ConverterConfiguration();
		dscc->append(configuration->getConfiguration());

		vector<string> expConfiguration = StringUtilities::tokenizeString((*experiment), ".");
		map<string, string> _ccm;
		_ccm.insert(_ccm.end(), make_pair("bruker.experiment.number", expConfiguration[0]));
		Logger::getInstance().debug("[BrukerPV3ToVistaConverter] setting bruker.experiment.number to " + expConfiguration[0]);
		
		if(expConfiguration.size() > 1) {
			_ccm.insert(_ccm.end(), make_pair("bruker.experiment.reco.number", expConfiguration[1]));
			Logger::getInstance().debug("[BrukerPV3ToVistaConverter] setting bruker.experiment.reco.number to " + expConfiguration[1]);
		} else {
			_ccm.insert(_ccm.end(), make_pair("bruker.experiment.reco.number", "1"));
			Logger::getInstance().debug("[BrukerPV3ToVistaConverter] setting bruker.experiment.reco.number to 1");
		}
		if(expConfiguration.size() > 2) {
			_ccm.insert(_ccm.end(), make_pair("bruker.experiment.reco.data.file", expConfiguration[2]));
			Logger::getInstance().debug("[BrukerPV3ToVistaConverter] setting bruker.experiment.reco.data.file to " + expConfiguration[2]);
		} else {
			_ccm.insert(_ccm.end(), make_pair("bruker.experiment.reco.data.file", "2dseq"));
			Logger::getInstance().debug("[BrukerPV3ToVistaConverter] setting bruker.experiment.reco.data.file to 2dseq");
		}

		dscc->append(_ccm);
		
		map<string, string>::iterator ccmi2 = dscc->getConfiguration().begin();
		while(ccmi2 != dscc->getConfiguration().end()) {
			Logger::getInstance().debug(ccmi2->first + " = " + ccmi2->second);
			ccmi2++;
		}
		
		
//		string _en, _ern, _erdf;
//		dscc->getConfigurationValue("bruker.experiment.number", _en);
//		dscc->getConfigurationValue("bruker.experiment.reco.number", _ern);
//		dscc->getConfigurationValue("bruker.experiment.reco.data.file", _erdf);
//		Logger::getInstance().debug("setting bruker.experiment.number: " + _en);
//		Logger::getInstance().debug("setting bruker.experiment.reco.number: " + _ern);
//		Logger::getInstance().debug("setting bruker.experiment.reco.data.file: " + _erdf);

		// create reader
		reader = new BrukerPV3Reader(dscc);

		// make the reader 'do its job'	
		datasets.push_back(reader->read());
		
		++experiment;
	}

	writer = new VistaWriter(configuration);
	writer->write(datasets);	
	
}





};
