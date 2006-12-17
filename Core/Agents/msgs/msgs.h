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

#ifndef		__MSGS_H__
#define		__MSGS_H__

#include <string>
#include <sstream>
#include <list>

#include "IBBWinAgent.h"

#include "EventLog.h"
#include "Logs.h"

#define	MAX_TABLE_PROC	1024

enum BBAlarmType { GREEN, YELLOW, RED };

class AgentMsgs : public IBBWinAgent
{
	private :
		IBBWinAgentManager 			& m_mgr;
		EventLog::Manager			m_eventlog;			// event log manager
		DWORD						m_delay;			// default delay used for match rules
		bool						m_alwaysgreen;		// no alerts if true
		std::string					m_testName;			// testname for the column
		Logs::Manager				m_logs;				// logs file manager

	private :
		void	AddEventLogRule(PBBWINCONFIGRANGE range, bool ignore, const std::string defLogFile);
		void	AddLogRule(PBBWINCONFIGRANGE range, bool ignore, const std::string defLogFile);
		void	AddRule(PBBWINCONFIGRANGE range, bool ignore, const std::string defLogFile);
		bool	LoadConfig(const std::string name);

	public :
		AgentMsgs(IBBWinAgentManager & mgr);
		~AgentMsgs();
		bool Init();
		void Run();
};


#endif 	// !__MSGS_H__

