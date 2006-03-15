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

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "IBBWinException.h"

#include "Singleton.h"

using namespace DesignPattern;

#define			LOGLEVEL_ERROR		1
#define			LOGLEVEL_WARN		2
#define 		LOGLEVEL_INFO		3
#define			LOGLEVEL_DEBUG		4

#define			LOGLEVEL_DEFAULT	4



//
// class used to log information
// singleton
class Logging : public Singleton< Logging > {
	private :
		DWORD					m_logLevel;
		std::string				m_fileName;
		std::ofstream 			*m_logFile;

		
	protected :
		void	open();
		void 	close();
		void 	write(const std::string & log);
		void 	reportEvent(WORD type, WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str);
	
	public :
		Logging();
		~Logging();
	
		void setLogLevel(DWORD level);
		void setFileName(const std::string & fileName);
		void logInfo(const std::string & log);
		void logWarn(const std::string & log);
		void logDebug(const std::string & log);
		void logError(const std::string & log);
		void reportInfoEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str);
		void reportSuccessEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str);
		void reportErrorEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str);
		void reportWarnEvent(WORD category, DWORD eventId, WORD nbStr, LPCTSTR *str);
		
};

/** class LoggingException 
*/
class LoggingException : public IBBWinException {
public:
	LoggingException(const char* m);
	std::string getMessage() const;
};	

#endif // __LOGGING_H__

