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


#ifndef __BBWINHANDLER_H__
#define __BBWINHANDLER_H__

#include "IBBWinException.h"
#include "BBWinAgentManager.h"
#include "IBBWinAgent.h"
#include "Logging.h"

#include "BBWinHandlerData.h"

// 
// inherit from the nice thread class written by Vijay Mathew Pandyalakal
// this class handle each agent and execute agent code
// 
class BBWinHandler : public Thread {

private:
	std::string			m_agentName;
	std::string			m_agentFileName;
	HMODULE  			m_hm;
	IBBWinAgent			*m_agent;
	HANDLE				*m_hEvents;
	DWORD				m_hCount;
	Logging				*m_log;
	bbdisplay_t			& m_bbdisplay;
	setting_t			& m_setting;
	DWORD				m_timer;
	BBWinAgentManager	*m_mgr;
	CREATEBBWINAGENT 	m_create;
	DESTROYBBWINAGENT 	m_destroy;

private:
	void	init();
	void 	checkAgentCompatibility();
	
public:
	BBWinHandler(bbwinhandler_data_t & data);
	
	void run();
};


/** class LoggingException 
*/
class BBWinHandlerException : IBBWinException {
public:
	BBWinHandlerException(const char* m);
	string getMessage() const;
};	

#endif // __BBWINHANDLER_H__
