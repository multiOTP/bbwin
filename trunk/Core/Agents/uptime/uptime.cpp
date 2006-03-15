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

#include <pdh.h>

#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>

#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "uptime.h"

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\009

#define SYSTEM_OBJECT_INDEX					2		// 'System' object
#define UPTIME_OBJECT_INDEX					674		// 'Uptime' object

const char				uptimeCounterPath[] = "\\\\.\\System\\System Up Time";

static const BBWinAgentInfo_t 		uptimeAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	0,              // agentMajVersion;
	2,              // agentMinVersion;
	"uptime",    // agentName;
	"uptime agent : report uptime server",        // agentDescription;
};                

const BBWinAgentInfo_t & AgentUptime::About() {
	return uptimeAgentInfo;
}

template <class T>
static T 			MyGetCounterValue(LPCTSTR counterPath) {
	T				value;
	PDH_STATUS  	status;
	HQUERY			perfQuery = NULL;
	HCOUNTER		uptimeCounter;
	
	PDH_FMT_COUNTERVALUE uptimeValue;
	value = 0;
	OSVERSIONINFO osvi;    
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(!GetVersionEx(&osvi) || (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT))
		return 0;//unknown OS or not NT based
	if( PdhOpenQuery( NULL, 0, &perfQuery ) != ERROR_SUCCESS )
		return 0;
	status = PdhAddCounter( perfQuery, counterPath,
							0, &uptimeCounter );
	if( status != ERROR_SUCCESS )
		return 0;
	status = PdhCollectQueryData( perfQuery );
	if( status != ERROR_SUCCESS )
		return 0;
	status = PdhGetFormattedCounterValue( uptimeCounter, PDH_FMT_LARGE , NULL, &uptimeValue );
	if( status != ERROR_SUCCESS )
		return 0;
	PdhRemoveCounter(uptimeCounter);
	PdhCloseQuery( perfQuery );
	value = (T) (uptimeValue.largeValue);
	return value;
}

void AgentUptime::Run() {
	DWORD				seconds;
	bool				alert = false;
	DWORD				day, hour, min;
	stringstream 		reportData;	
	
	seconds = MyGetCounterValue <DWORD> (uptimeCounterPath);
	reportData << "uptime status [" << m_mgr.GetSetting("hostname") << "]\n\n" << endl;
	if (seconds <= m_delay) {
		reportData << "&yellow machine rebooted recently" << endl;
		reportData << endl;
		alert = true;
	}
	day = seconds / 86400;
	seconds -= day * 86400;
	hour = seconds / 3600;
	seconds -= hour * 3600;
	min = seconds / 60;
	seconds -= min * 60;
	reportData << day  << " days " << hour << " hours " << min << " minutes " << seconds << " seconds" << endl;
	if (alert){
		m_mgr.Status("uptime", m_alarmColor, reportData.str());
	} else {
		m_mgr.Status("uptime", "green", reportData.str());
	}
}

bool AgentUptime::Init() {
	bbwinagentconfig_t		conf;
	
	if (m_mgr.LoadConfiguration(m_mgr.GetAgentName(), conf) == false)
		return false;
	std::pair< bbwinagentconfig_iter_t, bbwinagentconfig_iter_t > 	range;
	range = conf.equal_range("setting");
	for ( ; range.first != range.second; ++range.first) {
		if (range.first->second["name"] == "delay") {
			DWORD		seconds = m_mgr.GetSeconds(range.first->second["value"]);
			m_delay = seconds;
		}
		if (range.first->second["name"] == "alarmcolor") {
			m_alarmColor = range.first->second["value"];
		}
	}	
	return true;
}

AgentUptime::AgentUptime(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	m_delay = m_mgr.GetSeconds(UPTIME_DELAY);
	m_alarmColor = "yellow";
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentUptime(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
