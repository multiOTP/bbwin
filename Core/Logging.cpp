//this file is part of BBWin
//Copyright (C)2006 Etienne GRIGNON  ( etienne.grignon@gmail.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include <windows.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "BBWinService.h"

#include "logging.h"

CRITICAL_SECTION  		m_logCriticalSection;
CRITICAL_SECTION  		m_eventCriticalSection;

//
//  FUNCTION: Logging
//
//  PURPOSE: constructor
//
//  PARAMETERS:
//   none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// 
//
Logging::Logging() {
	InitializeCriticalSection(&m_logCriticalSection);
	InitializeCriticalSection(&m_eventCriticalSection);
	m_logLevel = LOGLEVEL_DEFAULT;
	m_logFile = new ofstream;
}

//
//  FUNCTION: Logging::setFileName
//
//  PURPOSE: set the filename
//
//  PARAMETERS:
//   fileName		file name of the log file
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// 
//
void Logging::setFileName(const std::string & fileName) {
	m_fileName = fileName;
}


//
//  FUNCTION: Logging::open
//
//  PURPOSE: open the file
//
//  PARAMETERS:
//   none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// 
//
void Logging::open() {
	m_logFile->open(m_fileName.c_str(), ios::out | ios::app);
	if (!m_logFile->is_open()) {
		throw LoggingException("Failed to create log file");
	}
}

//
//  FUNCTION: Logging::close
//
//  PURPOSE: close the file
//
//  PARAMETERS:
//   none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// 
//
void Logging::close() {
	m_logFile->close();
}


//
//  FUNCTION: ~Logging
//
//  PURPOSE: destructor
//
//  PARAMETERS:
//   none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// 
//
Logging::~Logging() {
	delete m_logFile;
	DeleteCriticalSection(&m_logCriticalSection);
	DeleteCriticalSection(&m_eventCriticalSection);
}

//
//  FUNCTION: Logging::write
//
//  PURPOSE: write the string to a file
//
//  PARAMETERS:
//   log         
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  the functions is implemented to work in multithreading environnment. 
//  that's why critical sections are used
//
void Logging::write(const string & log) {
	struct _SYSTEMTIME  ts;

	EnterCriticalSection(&m_logCriticalSection); 
    GetLocalTime(&ts);
	try {
		open();
	} catch (LoggingException ex) {
		LeaveCriticalSection(&m_logCriticalSection);
		return ;
	}
	*m_logFile << ts.wDay << "/" << ts.wMonth << "/" << ts.wYear << " " << ts.wHour; 
	*m_logFile << ":" << ts.wMinute << ":" << ts.wSecond << ":";
	*m_logFile << log << endl;
	close();
    LeaveCriticalSection(&m_logCriticalSection);
}

//
//  FUNCTION: Logging::logInfo
//
//  PURPOSE:  log information in the file
//
//  PARAMETERS:
//   log         log string
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  
//
void Logging::logInfo(const string & log) {
	if (m_logLevel >= LOGLEVEL_INFO) {
		string _log("[INFO]: ");
		
		_log += log;
		write(_log);
	}
}

//
//  FUNCTION: Logging::logError
//
//  PURPOSE:  log error in the file
//
//  PARAMETERS:
//   log         log string
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  
//
void Logging::logError(const string & log) {
	if (m_logLevel >= LOGLEVEL_ERROR) {
		string _log("[ERROR]: ");
		
		_log += log;
		write(_log);
	}
}

//
//  FUNCTION: Logging::logWarn
//
//  PURPOSE:  log warning in the file
//
//  PARAMETERS:
//   log         log string
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  
//
void Logging::logWarn(const string & log) {
	if (m_logLevel >= LOGLEVEL_WARN) {
		string _log("[WARNING]: ");
		
		_log += log;
		write(_log);
	}
}

//
//  FUNCTION: Logging::logDebug
//
//  PURPOSE:  log debug in the file
//
//  PARAMETERS:
//   log         log string
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  
//
void Logging::logDebug(const string & log) {
		if (m_logLevel >= LOGLEVEL_DEBUG) {
		string _log("[DEBUG]: ");
		
		_log += log;
		write(_log);
	}
}

//
//  FUNCTION: Logging::reportEvent
//
//  PURPOSE:  report an event to the log event
//
//  PARAMETERS:
//   type 		event type
//   category	bbwin categories
//   eventID	bbwin event id
//   nbStr		Nb of strings
//  str 		table of strings
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  
//
void Logging::reportEvent(WORD type, WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str) {
	HANDLE		hEvent;
	
	EnterCriticalSection(&m_eventCriticalSection); 
	if ((hEvent = RegisterEventSource(NULL, SZEVENTLOGNAME)) == NULL) {
		LeaveCriticalSection(&m_eventCriticalSection);
		return ;
	}
	ReportEvent(hEvent, type, category, eventId, NULL, nbStr, 0, str, NULL);
	DeregisterEventSource(hEvent);
	LeaveCriticalSection(&m_eventCriticalSection);
}


//
//  FUNCTION: Logging::report%%Event
//
//  PURPOSE:  report an event to the log event
//
//  PARAMETERS:
//   category	bbwin categories
//   eventID	bbwin event id
//   nbStr		Nb of strings
//  str 		table of strings
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  multiple public methods used for the developer to log events
//

void Logging::reportInfoEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str) {
	reportEvent(EVENTLOG_INFORMATION_TYPE, category, eventId, nbStr, str);
}

void Logging::reportSuccessEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str) {
	reportEvent(EVENTLOG_SUCCESS, category, eventId, nbStr, str);
}

void Logging::reportErrorEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str) {
	reportEvent(EVENTLOG_ERROR_TYPE, category, eventId, nbStr, str);
}

void Logging::reportWarnEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str) {
	reportEvent(EVENTLOG_WARNING_TYPE, category, eventId, nbStr, str);
}



void Logging::setLogLevel(DWORD level) {
	m_logLevel = level;
}

// LoggingException
LoggingException::LoggingException(const char* m) {
	msg = m;
}

string LoggingException::getMessage() const {
	return msg;
}
