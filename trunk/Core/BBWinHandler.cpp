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

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
using namespace std;

#include "ou_thread.h"
using namespace openutils;

#include "BBWinNet.h"
#include "BBWinHandler.h"
#include "BBWinMessages.h"

#include "Utils.h"
using namespace utils;

//
//  FUNCTION: BBWinHandler
//
//  PURPOSE: constructor
//
//  PARAMETERS:
//    
//    
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//
BBWinHandler::BBWinHandler(bbwinhandler_data_t & data) : 
							m_bbdisplay (data.bbdisplay), 
							m_setting (data.setting),
							m_agentName (data.agentName),
							m_agentFileName (data.agentFileName),
							m_hEvents (data.hEvents),
							m_hCount (data.hCount),
							m_timer (data.timer)
{
		Thread::setName(m_agentFileName.c_str());
		m_log = Logging::getInstancePtr();
		try {
			m_mgr = new BBWinAgentManager(data);
		} catch (std::bad_alloc ex) {
			throw ex;
		}
		if (m_mgr == NULL)
			throw std::bad_alloc("no more memory");
}

//
//  FUNCTION: BBWinHandler::init
//
//  PURPOSE: load the dll, get the adress procedure and instantiate the agent object
//
//  PARAMETERS:
//    none
//    
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  
//
void		BBWinHandler::init() {
	if ((m_hm = LoadLibrary(m_agentFileName.c_str())) == NULL) {
		string err;
		GetLastErrorString(err);
		LPCTSTR		arg[] = { m_agentFileName.c_str(), err.c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_LOAD_AGENT_ERROR, 2, arg);
		throw BBWinHandlerException("can't load agent");
	} 
	if ((m_create = (CREATEBBWINAGENT)GetProcAddress(m_hm, "CreateBBWinAgent")) == NULL) {
		string err;
		GetLastErrorString(err);
		LPCTSTR		arg[] = { m_agentFileName.c_str(), err.c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_LOAD_AGENT_ERROR, 2, arg);
		FreeLibrary(m_hm);
		throw BBWinHandlerException("can't get proc");
	}
	if ((m_destroy = (DESTROYBBWINAGENT)GetProcAddress(m_hm, "DestroyBBWinAgent")) == NULL) {
		string err;
		GetLastErrorString(err);
		LPCTSTR		arg[] = { m_agentFileName.c_str(), err.c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_LOAD_AGENT_ERROR, 2, arg);
		FreeLibrary(m_hm);
		throw BBWinHandlerException("can't get proc");
	}
	m_agent = m_create(*m_mgr);
	if (m_agent == NULL) {
		string err;
		GetLastErrorString(err);
		LPCTSTR		arg[] = { m_agentFileName.c_str(), err.c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_CANT_INSTANTIATE_AGENT, 2, arg);
		FreeLibrary(m_hm);
		throw BBWinHandlerException("can't allocate agent");
	}
}

//
//  FUNCTION: BBWinHandler::checkAgentCompatiblity
//
//  PURPOSE: it reports agent loading success for the moment
//
//  PARAMETERS:
//    none
//    
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  report loading success
// in future, this function will also check agent compatibility
//
void		BBWinHandler::checkAgentCompatibility() {
	const BBWinAgentInfo_t 	& info = m_agent->About();
	ostringstream 			oss;
	
	if (info.bbwinVersion != BBWIN_AGENT_VERSION) {
		// for future use
	}
    oss << endl;
	oss << "Agent name : " << info.agentName << endl;
	oss << "Agent version : " << info.agentMajVersion << "." << info.agentMinVersion << endl;
	oss << "BBWin agent version : " << info.bbwinVersion << endl;
	oss << "Agent description : " << info.agentDescription << endl;
	string result = oss.str();
	LPCTSTR		arg[] = {m_agentFileName.c_str(), result.c_str(), NULL};
	m_log->reportInfoEvent(BBWIN_SERVICE, EVENT_LOAD_AGENT_SUCCESS, 2, arg);
}

//
//  FUNCTION: BBWinHandler::run
//
//  PURPOSE: running part
//
//  PARAMETERS:
//    none
//    
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  thread run function. Thread is terminated at the end of this function
//
void BBWinHandler::run() {
	bool		initSuccess;
	string		mess;
	
	mess = "Thread for agent " + m_agentName + " started.";
	m_log->logDebug(mess);
	try {
		init();
	} catch (BBWinHandlerException ex) {
		m_log->logDebug("Init for agent failed.");
		return ;
	}
	checkAgentCompatibility();
	initSuccess = m_agent->Init();
	if (initSuccess) {
		for (;;) {
			DWORD 		dwWait;
			
			
			m_agent->Run();
			dwWait = WaitForMultipleObjects(m_hCount, m_hEvents , FALSE, m_timer * 1000 );
			if ( dwWait >= WAIT_OBJECT_0 && dwWait <= (WAIT_OBJECT_0 + m_hCount - 1) ) {    
				break ;
			} else if (dwWait >= WAIT_ABANDONED_0 && dwWait <= (WAIT_ABANDONED_0 + m_hCount - 1) ) {
				break ;
			}
		}
	}
	// be carefull with destroy call
	// problem with this call for the moment
	mess = "Agent " + m_agentName + " going to be destroyed.";
	m_log->logDebug(mess);
	m_destroy(m_agent);
	mess = "Agent " + m_agentName + " destroyed.";
	m_log->logDebug(mess);
	FreeLibrary(m_hm);
	mess = "Agent DLL " + m_agentName + " Unloaded.";
	m_log->logDebug(mess);
	delete m_mgr;
	mess = "Thread for agent " + m_agentName + " stopped.";
	m_log->logDebug(mess);
}

// BBWinHandlerException
BBWinHandlerException::BBWinHandlerException(const char* m) {
	msg = m;
}

string BBWinHandlerException::getMessage() const {
	return msg;
}
