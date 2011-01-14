/****************************************************************
 *
 * Program: brutov2
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Torsten Schlumm, 2001, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: brutov2.C 3774 2010-09-23 14:45:16Z tuerke $
 *
 *****************************************************************/


/* global includes */
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <cstdio>

/* local includes */
#include "Logger.h"
#include "OptionSet.h"
#include "OptionValues.h"
#include "IOption.h"
#include "StringOption.h"
#include "StringListOption.h"
#include "FileOption.h"
#include "DoubleOption.h"
#include "IntegerOption.h"
#include "ConverterConfiguration.h"
#include "ConverterFactory.h"
#include "Converter.h"


#define APPLICATION_NAME      "brutov2"
#define APPLICATION_VERSION   "3.0.3"
#define APPLICATION_COPYRIGHT "2005,2006,2008,2009,2010 Max-Planck-Institute for Human Cognitive and Brain Sciences"

extern "C" {
  extern void getLipsiaVersion(char*,size_t);
}

using namespace std;
using namespace option;
using namespace converter;
using namespace logger;



void printVersion() {
	stringstream _version;
	_version << "** " << APPLICATION_NAME << " v" << APPLICATION_VERSION << " (C) " << APPLICATION_COPYRIGHT << " **";
	// switch off timestamp and level printing for this one
	Logger::getInstance().setShowDateTime(false);
	Logger::getInstance().setShowLevel(false);
	Logger::getInstance().info("");
	Logger::getInstance().info(_version.str());
	Logger::getInstance().info("");
	// switch it on again
	Logger::getInstance().setShowDateTime(true);
	Logger::getInstance().setShowLevel(true);
}



int main(int argc, char** argv) {

        char prg_name[50];
        sprintf(prg_name,"brutov2 V%s", APPLICATION_VERSION);

        fprintf (stderr, "%s\n", prg_name);


	OptionSet* oSet = new OptionSet();

	// add all our options

	IOption* inputOption = new FileOption(
					"input-data",
					"in",
					"source location of the Bruker data to be converted (the top-level directory of the dataset e.g. /home/zoe/data/ZA2T050503.vu1)",
					"Bruker source data");
	inputOption->setAttribute("REQUIRED", "");
	inputOption->setAttribute("TYPE", "DIRECTORY");
	oSet->addOption(inputOption);

	IOption* outputOption = new FileOption(
					"output-data",
					"out",
					"target location of the converted Vista or Nifti data (e.g. /home/zoe/data/vista/ZA2T050503.vu1.2.v or /home/zoe/data/nifti/ZA2T050503.vu1.2.nii)",
					"converted Vista data");
	outputOption->setAttribute("REQUIRED", "");
	outputOption->setAttribute("TYPE", "FILE");
	oSet->addOption(outputOption);

	// this was ds before and is called scans in dictov
	IOption* expOption = new StringOption(
					"experiments",
					"exp",
					"the Bruker experiments to be converted (e.g. -exp 3, multiple experiments can be given: -exp 2,4,5)",
					"experiments to convert");
	expOption->setAttribute("REQUIRED", "");
	// expOption->setAttribute("REGEXP", ""); // this is for later on
	oSet->addOption(expOption);

	IOption* blackOption = new DoubleOption(
					"black",
					"b",
					"lower histogram cut-off value (percentage of the histogram area) 0.0 ... 100.0",
					"lower histogram cut-off");
	blackOption->setAttribute("MINIMUM", "0.0");
	blackOption->setAttribute("MAXIMUM", "100.0");
	blackOption->setAttribute("DEFAULT", "0.2");
	oSet->addOption(blackOption);

	IOption* whiteOption = new DoubleOption(
					"white",
					"w",
					"upper histogram cut-off value (percentage of the histogram area) 0.0 ... 100.0",
					"upper histogram cut-off");
	whiteOption->setAttribute("MINIMUM", "0.0");
	whiteOption->setAttribute("MAXIMUM", "100.0");
	whiteOption->setAttribute("DEFAULT", "0.2");
	oSet->addOption(whiteOption);

	IOption* verbosityOption = new StringListOption(
					"verbosity",
					"v",
					"level of output verbosity (debug | info | warning | error)",
					"verbosity");
	verbosityOption->setAttribute("DEFAULT", "warning");
	verbosityOption->setAttribute("HIDDEN", "");
	verbosityOption->setAttribute("LIST", "debug,info,warning,error");
	oSet->addOption(verbosityOption);

	IOption* precisionOption = new StringListOption(
					"precision",
					"p",
					"precision (bytes per voxel) of the created Vista data (auto | original | byte | short)",
					"precision");
	precisionOption->setAttribute("DEFAULT", "auto");
	precisionOption->setAttribute("HIDDEN", "");
	precisionOption->setAttribute("LIST", "auto,original,byte,short");
	oSet->addOption(precisionOption);

	IOption* nRepetitionsOption = new IntegerOption(
					"number-of-repetitions",
					"nrep",
					"the number of repetitions to convert (0 means all repetitions)",
					"repetitions to convert");
	nRepetitionsOption->setAttribute("MINIMUM", "0");
	nRepetitionsOption->setAttribute("HIDDEN", "");
	nRepetitionsOption->setAttribute("DEFAULT", "0");
	oSet->addOption(nRepetitionsOption);

	IOption* forceAnatomicalOption = new StringOption(
					"force-anatomical",
					"fa",
					"force the program to treat the input data as anatomical",
					"force anatomical data");
	forceAnatomicalOption->setAttribute("DEFAULT", "no");
	forceAnatomicalOption->setAttribute("HIDDEN", "");
	oSet->addOption(forceAnatomicalOption);

	IOption* forceFunctionalOption = new StringOption(
					"force-functional",
					"ff",
					"force the program to treat the input data as functional",
					"force functional data");
	forceFunctionalOption->setAttribute("DEFAULT", "no");
	forceFunctionalOption->setAttribute("HIDDEN", "");
	oSet->addOption(forceFunctionalOption);

	IOption* forceByteOrderOption = new StringListOption(
					"force-byteorder",
					"fbo",
					"force the program to use the given byte order (big | little)",
					"force wordtype");
	forceByteOrderOption->setAttribute("DEFAULT", "auto");
	forceByteOrderOption->setAttribute("HIDDEN", "");
	forceByteOrderOption->setAttribute("LIST", "auto,big,little");
	oSet->addOption(forceByteOrderOption);

	IOption* noNiftiOrientationOption = new StringOption(
					"no-nifti-orientation",
					"nno",
					"don't write orientation values (quaternions) into the Nifti header",
					"don't write Nifti orientation");
	noNiftiOrientationOption->setAttribute("DEFAULT", "no");
	noNiftiOrientationOption->setAttribute("HIDDEN", "");
	oSet->addOption(noNiftiOrientationOption);


	// initialize our logger
	Logger& logger = Logger::getInstance();
	logger.addStream(&cerr);
	logger.setLogLevel(Logger::INFO);

	// look at the options in a 'manual fashion' first to see if we've got
	// to give the user some help

	// simple help
	if(argc == 1 || (argc == 2 && (string(argv[1]).compare("-h") == 0 || string(argv[1]).compare("--help") == 0))) {
		printVersion();
		oSet->printHelp();
		exit(1);
	}

	// extended help (all the "black magic stuff")
	if(argc == 2 && (string(argv[1]).compare("-x") || string(argv[1]).compare("--extended-help"))) {
		printVersion();
		oSet->printExtendedHelp();
		exit(1);
	}

	// extract the commandline
	vector<string> _cLineVector;
	for(int i = 1; i < argc; i++) {
		_cLineVector.push_back(string(argv[i]));
	}

	OptionValues oValues;
	if(!oSet->parse(_cLineVector, oValues)) {
		// given options are incorrect - bail out
		exit(1);
	}

	string _vb = oValues.getOptionValue("verbosity");
	if(_vb.compare("debug") == 0) {
		logger.setLogLevel(Logger::DEBUG);
	} else if(_vb.compare("info") == 0) {
		logger.setLogLevel(Logger::INFO);
	} else if(_vb.compare("warning") == 0) {
		logger.setLogLevel(Logger::WARNING);
	} else if(_vb.compare("error") == 0) {
		logger.setLogLevel(Logger::ERROR);
	}

	printVersion();


	stringstream _msg;
	 _msg << " got " << oValues.size() << " option values";
	logger.debug(_msg.str());

	// do the converter stuff

	logger.info(" ... conversion started");

	// get converter factory
	ConverterFactory& cf = ConverterFactory::getInstance();

	// create a converter configuration
	map<string, string> ccm;
	ccm.insert(ccm.end(), make_pair(ConverterConfiguration::SOURCE_URL_KEY, "file:" + oValues.getOptionValue("input-data")));
	ccm.insert(ccm.end(), make_pair(ConverterConfiguration::TARGET_URL_KEY, "file:" + oValues.getOptionValue("output-data")));
	ccm.insert(ccm.end(), make_pair(ConverterConfiguration::SOURCE_FORMAT_KEY, ConverterConfiguration::BRUKER_FORMAT));
	// if 'output' ends with '.nii' we set the target format to Nifti
	if(oValues.getOptionValue("output-data").find(".nii", oValues.getOptionValue("output-data").length() - 4) != string::npos) {
		ccm.insert(ccm.end(), make_pair(ConverterConfiguration::TARGET_FORMAT_KEY, ConverterConfiguration::NIFTI_FORMAT));
	} else {
		ccm.insert(ccm.end(), make_pair(ConverterConfiguration::TARGET_FORMAT_KEY, ConverterConfiguration::VISTA_FORMAT));
	}

	ccm.insert(ccm.end(), make_pair(ConverterConfiguration::APPLICATION_NAME_KEY,    APPLICATION_NAME));
	ccm.insert(ccm.end(), make_pair(ConverterConfiguration::APPLICATION_VERSION_KEY, APPLICATION_VERSION));

	stringstream _closs;
	for(int i = 0; i < (int)_cLineVector.size(); i++) {
		_closs << _cLineVector[i] << " ";
	}
	ccm.insert(ccm.end(), make_pair(ConverterConfiguration::APPLICATION_COMMAND_LINE_OPTIONS_KEY, _closs.str()));

	ccm.insert(ccm.end(), make_pair("bruker.experiments", oValues.getOptionValue("experiments")));

	ccm.insert(ccm.end(), make_pair("histogram.cut.off.lower", oValues.getOptionValue("black")));
	ccm.insert(ccm.end(), make_pair("histogram.cut.off.upper", oValues.getOptionValue("white")));

	ccm.insert(ccm.end(), make_pair("output.precision", oValues.getOptionValue("precision")));

	ccm.insert(ccm.end(), make_pair("repetitions.to.convert", oValues.getOptionValue("number-of-repetitions")));

	ccm.insert(ccm.end(), make_pair("force.functional", oValues.getOptionValue("force-functional")));
	ccm.insert(ccm.end(), make_pair("force.anatomical", oValues.getOptionValue("force-anatomical")));

	ccm.insert(ccm.end(), make_pair("force.byteorder", oValues.getOptionValue("force-byteorder")));

	ccm.insert(ccm.end(), make_pair("no.nifti.orientation", oValues.getOptionValue("no-nifti-orientation")));

	ConverterConfiguration* cc = new ConverterConfiguration(ccm);

	// get the configured converter
	Converter* conv = cf.getConverter(cc);
	// check the converter
	if(!conv) {
		logger.error("Could not create converter. brutov2 exits.");
		return -1;
	}

	// and do it
	conv->convert();

	logger.info(" ... conversion finished");

	return 0;
}

