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

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "externals.h"

static const BBWinAgentInfo_t 		extAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	1,              					// agentMajVersion;
	0,              					// agentMinVersion;
	"externals",    					// agentName;
	"externals agent in charge to execute independant bb scripts",        // agentDescription;
};                

const BBWinAgentInfo_t & AgentExternals::About() {
	return extAgentInfo;
}

void 			AgentExternals::SendExternalReport(const string & reportName, const string & reportPath) {
	ifstream 	report(reportPath.c_str());
	string		res;
	
    if (report) {
		string 			statusLine; 
		stringstream 	reportData;	
        
		getline(report, statusLine );
		// check the validity of the line
		reportData << "status " << m_mgr.GetSetting("hostname") << "." << reportName << " ";
		reportData << statusLine << endl;
        reportData << report.rdbuf();
        //cout << "Buffer : \n" << reportData.str() << endl;
		report.close();
		m_mgr.Message(reportData.str(), res);
    }
}

void			AgentExternals::SendExternalsReports() {
	WIN32_FIND_DATA 	FindFileData;
	HANDLE 				hFind = INVALID_HANDLE_VALUE;
	string 				dirSpec;  // directory specification
	DWORD 				dwError;

	dirSpec += m_mgr.GetSetting("tmppath") + "\\*";
	hFind = FindFirstFile(dirSpec.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		string err;
		m_mgr.GetLastErrorString(err);
		string mess = "can't get file listing from tmp path " + m_mgr.GetSetting("tmppath") + ": " + err;
		m_mgr.ReportEventError(mess);
		return ;
	} else {
		string		reportName;
		string		reportPath;
		size_t		res;
		
		while (FindNextFile(hFind, &FindFileData) != 0) {
			reportName = FindFileData.cFileName;
			res = reportName.find(".");
			if (res < 0 || res > reportName.size()) {
				reportPath = m_mgr.GetSetting("tmppath") + "\\" + reportName;
				SendExternalReport(reportName, reportPath);
				DeleteFile(reportPath.c_str());
			}
		}
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			return ;
		}
	}
}

void 	AgentExternals::Run() {
	// launch externals
	vector<External *>::iterator		it;
	for (it = m_externals.begin(); it != m_externals.end(); ++it)  {
		(*it)->start();
	}
	for (;;) {
		DWORD 		dwWait;
		
		dwWait = WaitForMultipleObjects(m_hCount, m_hEvents , FALSE, m_logsTimer * 1000 );
		if ( dwWait >= WAIT_OBJECT_0 && dwWait <= (WAIT_OBJECT_0 + m_hCount - 1) ) {    
			break ;
		} else if (dwWait >= WAIT_ABANDONED_0 && dwWait <= (WAIT_ABANDONED_0 + m_hCount - 1) ) {
			break ;
		}
		SendExternalsReports();
	}
	for (it = m_externals.begin(); it != m_externals.end(); ++it)  {
		(*it)->stop();
	}
	for (it = m_externals.begin(); it != m_externals.end(); ++it)  {
		delete (*it);
	}
	m_externals.clear();
	delete [] m_hEvents;
}

AgentExternals::AgentExternals(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	m_timer = m_mgr.GetNbr(m_mgr.GetSetting("timer"));
	m_hCount = m_mgr.GetHandlesCount();
	m_hEvents = new HANDLE[m_hCount];
	m_mgr.GetHandles(m_hEvents);
	m_logsTimer = EXTERNAL_DEFAULT_LOG_TIMER;
}

bool		AgentExternals::Init() {
	if (m_mgr.LoadConfiguration(m_mgr.GetAgentName(), m_conf) == false)
		return false;
	
	std::pair< bbwinagentconfig_iter_t, bbwinagentconfig_iter_t > 	range;
	range = m_conf.equal_range("setting");
	for ( ; range.first != range.second; ++range.first) {
		if (range.first->second["name"] == "timer") {
			DWORD timer = m_mgr.GetSeconds(range.first->second["value"]);
			if (timer < 5 || timer > 2678400)
				m_mgr.ReportEventWarn("Setting timer must be between 5 seconds and 31 days(will use default setting (30 seconds) until you check your configuration)");
			else 
				m_timer = m_mgr.GetSeconds(range.first->second["value"]);
		}
		if (range.first->second["name"] == "logstimer") {
			DWORD logTimer = m_mgr.GetSeconds(range.first->second["value"]);
			if (logTimer < 5 || logTimer > 600)
				m_mgr.ReportEventWarn("Setting logstimer must be between 5 seconds and 10 minutes (will use default setting (30 seconds) until you check your configuration)");
			else 
				m_logsTimer = m_mgr.GetSeconds(range.first->second["value"]);
		}
	}	
	range = m_conf.equal_range("load");
	for ( ; range.first != range.second; ++range.first) {
		if (range.first->second["value"].length() > 0) {
			External		*ext;
			DWORD			timer = m_timer;
			if (range.first->second["timer"].length() > 0) {
				timer = m_mgr.GetSeconds(range.first->second["timer"]);
				if (timer < 5 || timer > 2678400) {
					string 	mess = "Setting timer ";
					mess += range.first->second["name"];
					mess += "must be between 5 seconds and 31 days(will use default setting (30 seconds) until you check your configuration)";
					m_mgr.ReportEventWarn(mess);
				} else {
					timer = m_mgr.GetSeconds(range.first->second["timer"]);
				}
			}
			ext = new External(m_mgr, range.first->second["value"], timer);
			m_externals.push_back(ext);
		}
	}
	if (m_externals.size() == 0) {
		m_mgr.ReportEventWarn("No externals have been specified");
	}
	return true;
}

void 	* AgentExternals::operator new (size_t sz) {
	return ::new unsigned char [sz];
}

void 	AgentExternals::operator delete (void * ptr) {
	::delete [] (unsigned char *)ptr;
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentExternals(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
