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
#include <assert.h>

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
							m_timer (data.timer),
							m_conf (NULL)
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
	
	assert(hEvents != NULL);
	for (ci = 0; ci < m_hCount; ++ci) {
		hEvents[ci] = m_hEvents[ci];
	}
}

DWORD			BBWinAgentManager::GetSeconds(LPCTSTR str) {
	string		temp;

	assert(str != NULL);
	assert(str != NULL);
	temp = str;
	return utils::GetSeconds(temp);
}

void			BBWinAgentManager::GetLastErrorString(LPTSTR dest, DWORD size) {
	string		sDest;

	assert(dest != NULL);
	utils::GetLastErrorString(sDest);
	strncpy(dest, sDest.c_str(), size);
} 

void			BBWinAgentManager::GetEnvironmentVariable(LPCTSTR varname, LPTSTR dest, DWORD size) {
	string		sDest;

	assert(dest != NULL);
	utils::GetEnvironmentVariable(varname, sDest);
	strncpy(dest, sDest.c_str(), size);
}

DWORD			BBWinAgentManager::GetNbr(LPCTSTR str) {
	string		temp;

	assert(str != NULL);
	temp = str;
	return utils::GetNbr(temp);
}

const std::string	& BBWinAgentManager::GetSetting(LPCTSTR settingName) {
	assert(settingName != NULL);
	return m_setting[ settingName]; 
}

void			BBWinAgentManager::LoadFileConfiguration(const string & filePath, const string & nameSpace, bbwinagentconfig_t & config) {
	BBWinConfig					*conf = BBWinConfig::getInstancePtr();
	try {	
		conf->LoadConfiguration(filePath, nameSpace, config);
	} catch (BBWinConfigException ex) {
		throw ex;
	}
}

bbwinagentconfig_t * BBWinAgentManager::LoadConfiguration(LPCTSTR nameSpace) {
	string		filePath;
	
	assert(nameSpace != NULL);
	if (m_conf != NULL) {
		return m_conf;
	}
	m_conf = new bbwinagentconfig_t;
	if (m_conf == NULL)
		return NULL;
	filePath = m_setting[ "etcpath" ];
	filePath += "\\";
	filePath += m_setting[ "bbwinconfigfilename" ]; 
	try {
		LoadFileConfiguration(filePath, nameSpace, *m_conf);
	} catch (BBWinConfigException ex) {
		return m_conf;
	}
	return m_conf;
}



bbwinagentconfig_t * BBWinAgentManager::LoadConfiguration(LPCTSTR fileName, LPCTSTR nameSpace) {
	string		filePath;
	
	assert(nameSpace != NULL);
	if (m_conf != NULL) {
		return m_conf;
	}
	m_conf = new bbwinagentconfig_t;
	if (m_conf == NULL)
		return NULL;
	filePath = m_setting[ "etcpath" ];
	filePath += "\\";
	filePath += fileName; 
	try {
		LoadFileConfiguration(filePath, nameSpace, *m_conf);
	} 	catch (BBWinConfigException ex) {
		return m_conf;
	}
	return m_conf;
}

void			BBWinAgentManager::FreeConfiguration(bbwinagentconfig_t * conf) {
	// conf argument not used for the moment because we only handle loading configuration one by one
	if (m_conf != NULL) {
		delete m_conf;
		m_conf = NULL;
	}
}


bbwinconfig_range_t * BBWinAgentManager::GetConfigurationRange(bbwinagentconfig_t * conf, LPCTSTR name) {
	// range configuration one by one for the moment
	assert(conf != NULL);
	assert(m_conf != NULL);
	m_range = m_conf->equal_range(name);
	return &m_range;
}

LPCTSTR				BBWinAgentManager::GetConfigurationRangeValue(bbwinconfig_range_t *range, LPCTSTR name) {
	assert(range != NULL);
	assert(name != NULL);
	if (range->first->second[name].size() == 0)
		return "";
	return range->first->second[name].c_str();
}

void				BBWinAgentManager::FreeConfigurationRange(bbwinconfig_range_t *range) {
	// range configuration one by one for the moment
	assert(range != NULL);
}


// Report Functions : report in the bbwin log file
void 	BBWinAgentManager::ReportError(LPCTSTR str) {
	string 		log;
	
	assert(str != NULL);
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logError(log);
}

void 	BBWinAgentManager::ReportInfo(LPCTSTR str) {
	string 	log;
	
	assert(str != NULL);
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logInfo(log);
}

void 	BBWinAgentManager::ReportDebug(LPCTSTR str) {
	string 	log;
	
	assert(str != NULL);
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logDebug(log);
}

void 	BBWinAgentManager::ReportWarn(LPCTSTR str) {
	string 	log;
	
	assert(str != NULL);
	log += "[";
	log += m_agentName;
	log += "]: ";
	log += str;
	m_log->logWarn(log);
}

// Event Report Functions : report in the Windows event log
void 	BBWinAgentManager::ReportEventError(LPCTSTR str) {
	assert(str != NULL);
	LPCTSTR		arg[] = {m_agentName.c_str(), str, NULL};
	
	m_log->reportErrorEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
} 

void 	BBWinAgentManager::ReportEventInfo(LPCTSTR str) {
	assert(str != NULL);
	LPCTSTR		arg[] = {m_agentName.c_str(), str, NULL};
	
	m_log->reportInfoEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
}

void 	BBWinAgentManager::ReportEventWarn(LPCTSTR str) {
	assert(str != NULL);
	LPCTSTR		arg[] = {m_agentName.c_str(), str, NULL};
	
	m_log->reportWarnEvent(BBWIN_AGENT, EVENT_MESSAGE_AGENT, 2, arg);
}

void 			BBWinAgentManager::Status(LPCTSTR testName, LPCTSTR color, LPCTSTR text, LPCTSTR lifeTime) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
	
	assert(testName != NULL);
	assert(color != NULL);
	assert(text != NULL);
	assert(lifeTime != NULL);
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

void		BBWinAgentManager::Notify(LPCTSTR testName, LPCTSTR text) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	assert(testName != NULL);
	assert(text != NULL);
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

void		BBWinAgentManager::Data(LPCTSTR dataName, LPCTSTR text) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
		
	assert(dataName != NULL);
	assert(text != NULL);
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

void 		BBWinAgentManager::Disable(LPCTSTR testName, LPCTSTR duration, LPCTSTR text) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	assert(testName != NULL);
	assert(duration != NULL);
	assert(text != NULL);
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

void		BBWinAgentManager::Enable(LPCTSTR testName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	assert(testName != NULL);
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


void		BBWinAgentManager::Drop(LPCTSTR testName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	assert(testName != NULL);
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

void		BBWinAgentManager::Rename(LPCTSTR newHostName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	assert(newHostName != NULL);
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


void		BBWinAgentManager::Rename(LPCTSTR oldTestName, LPCTSTR newTestName) {
	bbdisplay_t::iterator			itr;
	BBWinNet						hobNet;
		
	assert(oldTestName != NULL);
	assert(newTestName != NULL);
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

void		BBWinAgentManager::Message(LPCTSTR message, LPTSTR dest, DWORD size) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
	string		result;

	assert(message != NULL);
	assert(dest != NULL);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Message(message, result);
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
	strncpy(dest, result.c_str(), size);
}

void		BBWinAgentManager::Config(LPCTSTR fileName, LPTSTR dest, DWORD size) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
	string		result;

	assert(fileName != NULL);
	assert(dest != NULL);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Config(fileName, result);
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
	strncpy(dest, result.c_str(), size);
}

void		BBWinAgentManager::Query(LPCTSTR testName, LPTSTR dest, DWORD size) {
	bbdisplay_t::iterator			itr;
	BBWinNet	hobNet;
	string		result;

	assert(testName != NULL);
	assert(dest != NULL);
	for ( itr = m_bbdisplay.begin(); itr != m_bbdisplay.end(); ++itr) {
		hobNet.SetBBDisplay((*itr));
		try {
			hobNet.Query(testName, result);
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
	strncpy(dest, result.c_str(), size);
}
