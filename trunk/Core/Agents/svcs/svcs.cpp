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

#include <iostream>
#include <sstream>
#include <fstream>

#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"

using namespace boost::posix_time;
using namespace boost::gregorian;

#include "svcs.h"

static const BBWinAgentInfo_t 		svcsAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	0,              // agentMajVersion;
	1,              // agentMinVersion;
	"svcs",    // agentName;
	"svcs agent : check Windows services",        // agentDescription;
};                

//
// common bb colors
//
static const char	*bbcolors[] = {"green", "yellow", "red", NULL};


static const struct _svcStatus {
	LPCSTR		name;
	DWORD		status;
} svcStatusTable [] = {
	{"continue pending", SERVICE_CONTINUE_PENDING},
	{"pause pending", SERVICE_PAUSE_PENDING},
	{"paused", SERVICE_PAUSED},
	{"started", SERVICE_RUNNING},
	{"start pending", SERVICE_START_PENDING},
	{"stop pending", SERVICE_STOP_PENDING},
	{"stopped", SERVICE_STOPPED},
	{NULL, 0}
};

LPCSTR		findSvcStatus(DWORD status) {
	for (DWORD inc = 0; svcStatusTable[inc].name != NULL; inc++) {
		if (svcStatusTable[inc].status == status)
			return svcStatusTable[inc].name;
	}
	return NULL;
}

//
//
// SvcRule Constructor
//
SvcRule::SvcRule() :
		m_name(""),
		m_alarmColor(YELLOW), 
		m_state(SERVICE_RUNNING),
		m_reset(false)
{
	
}

//
//
// SvcRule Copy Constructor
//
SvcRule::SvcRule(const SvcRule & rule) {
	m_alarmColor = rule.GetAlarmColor();
	m_state = rule.GetSvcState();
	m_name = rule.GetName();
	m_reset = rule.GetReset();
}


//
// Agent functions

const BBWinAgentInfo_t & AgentSvcs::About() {
	return svcsAgentInfo;
}

#define 	MAX_SERVICE_LENGTH		1024


void					AgentSvcs::CheckSimpleService(SC_HANDLE & scm, LPCTSTR name, stringstream & reportData) {
	SC_HANDLE 				scs;
	LPQUERY_SERVICE_CONFIG 	lpServiceConfig;
	SERVICE_STATUS 			servStatus;
	DWORD 					bytesNeeded;
	BOOL 					retVal;
	TCHAR					tempName[MAX_SERVICE_LENGTH + 1];
	
	bytesNeeded = MAX_SERVICE_LENGTH;
	if ((GetServiceKeyName(scm, name, tempName, &bytesNeeded)) == 0) {
		return ;
	}
	tempName[bytesNeeded] = '\0';
	scs = OpenService(scm, tempName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP);
	if (scs == NULL) {
		string		sName = tempName;
		string 		mess = "can't open service" + sName;
		m_mgr.ReportInfo(mess.c_str());
		return ;
	}
	retVal = QueryServiceConfig(scs, NULL, 0, &bytesNeeded);
	DWORD err = GetLastError();
	if ((retVal == FALSE) || err == ERROR_INSUFFICIENT_BUFFER) {
		DWORD dwBytes = bytesNeeded + sizeof(QUERY_SERVICE_CONFIG);
		
		try {
			lpServiceConfig = (LPQUERY_SERVICE_CONFIG)new unsigned char [dwBytes];
		} catch (std::bad_alloc ex) {
			CloseServiceHandle(scs);
			return ;
		}
		if (lpServiceConfig == NULL) {
			CloseServiceHandle(scs);
			return ;
		}
		if (QueryServiceConfig(scs, lpServiceConfig, dwBytes, &bytesNeeded) != 0 && 	
				QueryServiceStatus(scs, &servStatus) != 0) {
					map<string, SvcRule>::iterator	itr;

					itr = m_rules.find(name);
					if (itr != m_rules.end()) { // treat the defined rule
						SvcRule	& rule = itr->second;
						if (rule.GetSvcState() != servStatus.dwCurrentState) {
							if (servStatus.dwCurrentState == SERVICE_STOPPED) { // service is stopped and should be running
								reportData << "&" << bbcolors[rule.GetAlarmColor()] << " " << name << " is stopped";
								if (rule.GetReset()) {
									StartService(scs, 0, NULL);
									reportData << " and will be restarted automatically" << endl;
								}
							} else if (servStatus.dwCurrentState == SERVICE_RUNNING) { // service is running and should be stopped
								SERVICE_STATUS		st;

								reportData << "&" << bbcolors[rule.GetAlarmColor()] << " " << name << " is running";
								if (rule.GetReset()) {
									ControlService(scs, SERVICE_CONTROL_STOP, &st);
									 reportData << " and will be stopped automatically" << endl;
								}
							} else {
								LPCSTR	str = (itr->second.GetSvcState() == SERVICE_RUNNING) ? "running" : "stoppped";
								reportData << "&" << bbcolors[rule.GetAlarmColor()] << " " << name << " should be " 
									<< str << " and is in an unmanaged state (" << findSvcStatus(servStatus.dwCurrentState) << ")";
							}
							if (m_pageColor < itr->second.GetAlarmColor())
								m_pageColor = itr->second.GetAlarmColor();
							reportData << endl;
						} else { // service is ok
							LPCSTR		str = (servStatus.dwCurrentState == SERVICE_RUNNING) ? "running" : "stoppped";
							reportData << "&" << bbcolors[GREEN] << " " << name << " is ";
							reportData << str << endl;
						}
					} else {
						if (m_autoReset) {
							if (servStatus.dwCurrentState == SERVICE_STOPPED 
								&& lpServiceConfig->dwStartType == SERVICE_AUTO_START) {
								StartService(scs, 0, NULL);
								reportData << "&" << bbcolors[m_alarmColor] << " " << name << " is stopped and will be restarted automatically" << endl;
								if (m_pageColor < m_alarmColor)
									m_pageColor = m_alarmColor;
							} else if (servStatus.dwCurrentState != SERVICE_RUNNING 
								&& lpServiceConfig->dwStartType == SERVICE_AUTO_START) {
								reportData << "&" << bbcolors[m_alarmColor] << " " << name
									<< " should be started and is in an unmanaged state (" << findSvcStatus(servStatus.dwCurrentState) << ")" << endl;
								if (m_pageColor < m_alarmColor)
									m_pageColor = m_alarmColor;
							}
						}
					}
		}
		delete [] lpServiceConfig;
	}
	CloseServiceHandle(scs);
}


// 
// CheckServices method
void				AgentSvcs::CheckServices(stringstream & reportData) {
	SC_HANDLE 				scm;
	ENUM_SERVICE_STATUS 	service_data, *lpservice;
	DWORD 					bytesNeeded, srvCount, resumeHandle = 0;
	BOOL 					retVal;
	
	m_mgr.ReportDebug("OpenSCManager");
	if ((scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE)) == NULL) {
		return ;
	}
	retVal = ::EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, &service_data, sizeof(service_data),
						&bytesNeeded, &srvCount, &resumeHandle);
	DWORD err = GetLastError();
    //Check if EnumServicesStatus needs more memory space
    if ((retVal == FALSE) || err == ERROR_MORE_DATA) {
		DWORD dwBytes = bytesNeeded + sizeof(ENUM_SERVICE_STATUS);
        
		try {
			lpservice = new ENUM_SERVICE_STATUS [dwBytes];
		} catch (std::bad_alloc ex) {
			CloseServiceHandle(scm);
		}
        EnumServicesStatus (scm, SERVICE_WIN32, SERVICE_STATE_ALL, lpservice, dwBytes,
								&bytesNeeded, &srvCount, &resumeHandle);
        for(DWORD inc = 0; inc < srvCount; inc++) {
			CheckSimpleService(scm, lpservice[inc].lpDisplayName, reportData);
		}
		delete [] lpservice;
    }
	m_mgr.ReportDebug("CloseServiceHandle");
	CloseServiceHandle(scm);
}

//
// Run method
// called from init
//
void AgentSvcs::Run() {
	stringstream 					reportData;	
	
    ptime now = second_clock::local_time();
	m_pageColor = GREEN;
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "] ";
	if (m_rules.size() == 0)
		reportData << "No service to check";
	if (m_autoReset) {
		reportData << " - Autoreset is On";
	}
	reportData << "\n" << endl;
	CheckServices(reportData);
	reportData << endl;
	if (m_alwaysGreen)
		m_pageColor = GREEN;
	m_mgr.Status(m_testName.c_str(), bbcolors[m_pageColor], reportData.str().c_str());
}

//
// init function
//
bool AgentSvcs::Init() {
	bbwinagentconfig_t		*conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());
	
	if (conf == NULL)
		return false;
	bbwinconfig_range_t * range = m_mgr.GetConfigurationRange(conf, "setting");
	if (range == NULL)
		return false;
	for ( ; range->first != range->second; ++range->first) {
		string		name, value;

		name = m_mgr.GetConfigurationRangeValue(range, "name");
		value = m_mgr.GetConfigurationRangeValue(range, "value");
		if (name == "alwaysgreen") {
			if (value == "true")
				m_alwaysGreen = true;
		} else if (name == "autoreset") {
			if (value == "true")
				m_autoReset = true;
		} else if (name == "testname") {
			if (value.length() > 0)
				m_testName = value;
		} else if (name == "alarmcolor") {
			if (value.length() > 0) {
				if (value == "red") 
					m_alarmColor = RED;
				else if (value == "yellow") 
					m_alarmColor = YELLOW;
				else if (value == "green") 
					m_alarmColor = GREEN;
			}
		} else {
			string	autoreset = m_mgr.GetConfigurationRangeValue(range, "autoreset");
			string	color = m_mgr.GetConfigurationRangeValue(range, "alarmcolor");

			if (name.length() > 0 && value.length() > 0) {
				SvcRule		rule;
				map<string, SvcRule>::iterator		itr;

				rule.SetName(name);
				if (autoreset.length() > 0 && autoreset == "true") {
					rule.SetReset(true);
				}
				if (value.length() > 0 && value == "stopped") {
					rule.SetSvcState(SERVICE_STOPPED);
				} else  {
					rule.SetSvcState(SERVICE_RUNNING);
				}
				if (color.length() > 0) {
					if (color == "red") 
						rule.SetAlarmColor(RED);
					else if (color == "yellow") 
						rule.SetAlarmColor(YELLOW);
					else if (color == "green") 
						rule.SetAlarmColor(GREEN);
				}
				itr = m_rules.find(name);
				if (itr == m_rules.end()) {
					m_rules.insert(pair<string, SvcRule>(name, rule));
				}
			}
		}
	}
	m_mgr.FreeConfigurationRange(range);
	m_mgr.FreeConfiguration(conf);
	return true;
}


//
// constructor 
//
AgentSvcs::AgentSvcs(IBBWinAgentManager & mgr) : 
		m_mgr(mgr),
		m_alarmColor(YELLOW),
		m_pageColor(GREEN),
		m_alwaysGreen(false),
		m_autoReset(false)
{
	m_testName = "svcs";
}

//
// Destructor 
//
AgentSvcs::~AgentSvcs() {
}


//
// common agents export functions
//

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentSvcs(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
