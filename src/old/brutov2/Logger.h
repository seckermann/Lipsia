#ifndef _LOGGER_H_
#define _LOGGER_H_

/* global includes */
#include <iostream>
#include <ostream>
#include <vector>

/* local includes */

namespace logger
{

class Logger
{
public:
	enum LogLevel { DEBUG, INFO, WARNING, ERROR };

	inline static Logger &getInstance() {
		static Logger instance;
		return instance;
	}



	virtual void addStream( std::ostream *stream );

	virtual void debug( std::string message );
	virtual void info( std::string message );
	virtual void warning( std::string message );
	virtual void error( std::string message );

	virtual LogLevel getLogLevel();
	virtual LogLevel setLogLevel( LogLevel level );

	virtual bool getShowDateTime();
	virtual bool setShowDateTime( bool showDateTime );

	virtual bool getShowLevel();
	virtual bool setShowLevel( bool showLevel );


private:
	Logger();
	virtual ~Logger();

	virtual void log( std::string message, LogLevel level );

	LogLevel logLevel;

	bool showDateTime;
	bool showLevel;


	std::vector<std::ostream *> streams;
};

static const std::string logLevelNames[] = { "DEBUG", "INFO", "WARNING", "ERROR" };
};

#endif /*_LOGGER_H_*/
