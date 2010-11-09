/* global includes */
#include <ctime>

/* local includes */
#include "Logger.h"


using namespace std;

namespace logger
{

Logger::Logger() {
	showDateTime = true;
	showLevel    = true;
}

Logger::~Logger() {
}


Logger::LogLevel Logger::setLogLevel(LogLevel level) {
	Logger::LogLevel _oldLevel = logLevel;
	logLevel = level;
	return _oldLevel;
}

bool Logger::setShowDateTime(bool showDT) {
	bool _oldShowDT = showDateTime;
	showDateTime = showDT;
	return _oldShowDT;
}

bool Logger::setShowLevel(bool showL) {
	bool _oldShowL = showLevel;
	showLevel = showL;
	return _oldShowL;
}

Logger::LogLevel Logger::getLogLevel() {
	return logLevel;
}

bool Logger::getShowDateTime() {
	return showDateTime;
}

bool Logger::getShowLevel() {
	return showLevel;
}


void Logger::addStream(ostream* stream) {
	streams.push_back(stream);
}

void Logger::debug(string message) {
	log(message, DEBUG);
}

void Logger::info(string message) {
	log(message, INFO);
}

void Logger::warning(string message) {
	log(message, WARNING);
}

void Logger::error(string message) {
	log(message, ERROR);
}

void Logger::log(string message, LogLevel level) {
	if(logLevel <= level) {
		vector<ostream*>::iterator sIterator = streams.begin();
		while(sIterator != streams.end()) {
			if(showDateTime) {
				struct tm *currentTime;
				time_t _t = std::time(NULL);
				currentTime = std::localtime(&_t);
				char _timeString[100];
				std::strftime(_timeString, sizeof(_timeString), "%Y-%m-%d %H:%M:%S", currentTime);
				(**sIterator) << _timeString << " ";
			}
			if(showLevel) {
				(**sIterator) << "[" << logLevelNames[level] << "]" << " ";
			}
			
			(**sIterator) << "\t" << message << endl;
			++sIterator;
		}
	}
}

};
