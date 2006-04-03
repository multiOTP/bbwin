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
#include <iostream>
#include <sstream>
using namespace std;

#include "BBWinNet.h"
#include "BBWinAgentManager.h"
#include "BBWinConfig.h"
#include "BBWinMessages.h"

#include "Utils.h"

//
//  FUNCTION: BBWinAgentManager
//
//  PURPOSE: constructor
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    bbwinhandler_data_t 		data structure containing all necessary data for the Agent Manager
//
//  COMMENTS:
// Init all members values
//
BBWinAgentManager::BBWinAgentManager(const bbwinhandler_data_t & data) : 
							m_bbdisplay (data.bbdisplay), 
							m_setting (data.setting),
							m_agentName (data.agentName),
							m_agentFileName (data.agentFileName),
							m_hEvents (data.hEvents),
							m_hCount (data.hCount),
							m_timer (data.timer)
{
		m_log = Logging::getInstancePtr();
		m_logReportFailure = false;
		if (m_setting["logreportfailure"] == "true")
			m_logReportFailure = true;	
}

//
//  FUNCTION: BBWinAgentManager::GetHandles
//
//  PURPOSE:  get current handles
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//
void	BBWinAgentManager::GetHandles(HANDLE * hEvents)	{
	DWORD			ci;
	
	for (ci = 0; ci < m_hCount; ++ci) {
		hEvents[ci] = m_hEvents[ci];
	}
}

DWORD			BBWinAgentManager::GetSeconds(const string & str) {
	return utils::GetSeconds(str);
}

void			BBWinAgentManager::GetLastErrorString(string & str) {
	utils::GetLastErrorString(str);
} 

void			BBWinAgentManager::GetEnvironmentVariable(const string & varname, string & dest) {
	utils::GetEnvironmentVariable(varname, dest);
}

DWORD			BBWinAgentManager::GetNbr(const string & str ) {
	return utils::GetNbr(str);
}


void			BBWinAgentManager::LoadFileConfiguration(const string & filePath, const string & nameSpace, bbwinagentconfig_t & config) {
	BBWinConfig					*conf = BBWinConfig::getInstancePtr();
	try {	
		conf->LoadConfiguration(filePath, nameSpace, config);
	} catch (BBWinConfigException ex) {
		throw ex;
	}
}

bool			BBWinAgentManager::LoadConfiguration(const string & nameSpace, bbwinagentconfig_t & config) {
	string		filePath;
	
	filePath = m_setting[ "etcpath" ];
	filePath += "\\";
	filePath += m_setting[ "bbwinconfigfilename" ]; 
	try {
		LoadFileConfiguration(filePath, nameSpace, config);
	} catch (BBWinConfigException ex) {
		return false;
	}
	return true;
}

bool			BBWinAgentManager::LoadConfiguration(const string & fileName, const string & nameSpace, bbwinagentconfig_t & config) {
	string		filePath;
	
	filePath = m_setting[ "etcpath" ];
	filePath += "\\";
	filePath += fileName; 
	try {
		LoadFileConfiguration(filePath, nameSpace, config);
	} 	catch (BBWinConfigException ex) {
		return false;
	}
	return true;
}


// Report Functions : report in the bbwin log file
void 	BBWinAgentManager::ReportError(const string & str) {
	string 		log;
	
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logError(log);
}

void 	BBWinAgentManager::ReportInfo(const string & str) {
	string 	log;
	
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logInfo(log);
}

void 	BBWinAgentManager::ReportDebug(const string & str) {
	string 	log;
	
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logDebug(log);
}

void 	BBWinAgentManager::ReportWarn(const string & str) {
	string 	log;
	
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logWarn(log);
}

// Event Report Functions : report in the Windows event log
void 	BBWinAgentManager::ReportEventError(const string & str) {
	LPCTSTR		arg[] = {m_agentName.c_str(), str.c_str(), NULL};
	
	m_log->reportErrorEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
} 

void 	BBWinAgentManager::ReportEventInfo(const string & str) {
	LPCTSTR		arg[] = {m_agentName.c_str(), str.c_str(), NULL};
	
	m_log->reportInfoEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
}

void 	BBWinAgentManager::ReportEventWarn(const string & str) {
	LPCTSTR		arg[] = {m_agentName.c_str(), str.c_str(), NULL};
	
	m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
}

void 			BBWinAgentManager::Status(const string & testName, const string & color, const string & text, const string & lifeTime) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Status(testName, color, text, lifeTime);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
						
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Notify(const string & testName, const string & text) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Notify(testName, text);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Data(const string & dataName, const string & text) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Data(dataName, text);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void 		BBWinAgentManager::Disable(const string & testName, const string & duration, const string & text) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Disable(testName, duration, text);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Enable(const string & testName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Enable(testName);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Drop()  {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Drop();
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}


void		BBWinAgentManager::Drop(const string & testName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Drop(testName);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Rename(const string & newHostName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Rename(newHostName);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}


void		BBWinAgentManager::Rename(const string & oldTestName, const string & newTestName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	hobNet.SetHostName(m_setting["hostname"]);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Rename(oldTestName, newTestName);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Message(const string & message, string & dest) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Message(message, dest);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Config(const string & fileName, string & dest) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Config(fileName, dest);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}

void		BBWinAgentManager::Query(const string & testName, string & dest) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Query(testName, dest);
		} catch (BBWinNetException ex) {
			if (m_logReportFailure) {
				string mes;
			
				mes = "Sending report to " + (*itr) + " failed.";
				LPCTSTR		arg[] = {m_agentName.c_str(), mes.c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
			}
			continue ; 
		}
	}
}
