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

#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>

#include <Psapi.h>

#include <string>

using namespace std;

#include "SystemCounters.h"


#define BBWIN_AGENT_EXPORTS

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"

using boost::format;
using namespace boost::posix_time;
using namespace boost::gregorian;


#include "CpuUsage.h"
#include "cpu.h"


static const char *bbcolors[] = { "green", "yellow", "red", NULL };


static const BBWinAgentInfo_t 		cpuAgentInfo =
{
	BBWIN_AGENT_VERSION,					// bbwinVersion;
	0,              						// agentMajVersion;
	1,              						// agentMinVersion;
	"cpu",    								// agentName;
	"cpu agent : report cpu usage",        	// agentDescription;
};                

const BBWinAgentInfo_t & AgentCpu::About() {
	return cpuAgentInfo;
}



//
// return the number of processor
// return 1 if the information can't be resolved
//
static DWORD	CountProcessor() {
	HKEY 		hKey;
	DWORD 		count;
	
	count = 0;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor"), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey) != ERROR_SUCCESS) {		
		return 1;
	}
	RegQueryInfoKey(hKey, NULL, NULL, NULL, &count, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	RegCloseKey(hKey);
	if (count == 0)
		count = 1;
	return count;
}

//
// return count number
//
static DWORD 		countProcesses() {
    DWORD 		aProcesses[MAX_TABLE_PROC], cbNeeded, cProcesses;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return 0;
    cProcesses = cbNeeded / sizeof(DWORD);
	return cProcesses;
}

//**********************************
//
// UsageProc class Methods
//
//**********************************

//
// constructor
UsageProc::UsageProc(DWORD pid) {
	m_pid = pid;
	m_exists = false;
	m_lastUsage = 0.00;
}

//
// Get Usage function
//
double			UsageProc::ExecGetUsage() {
	m_lastUsage = m_usage.GetCpuUsage(m_pid);
	return m_lastUsage;
}

//**********************************
//
// CPU agent class Methods
//
//**********************************

void 		AgentCpu::GetProcsData() {
	static DWORD 		aProcesses[MAX_TABLE_PROC], cbNeeded, cProcesses;
	
	ZeroMemory(aProcesses, sizeof(aProcesses));
	m_mgr.ReportDebug("GetProcsData started");
    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return ;
    cProcesses = cbNeeded / sizeof(DWORD);
    for (DWORD inc = 0; inc < cProcesses; ++inc ) {
		DWORD								pid = aProcesses[inc];
		procs_itr							itr;
		TCHAR 								szProcessName[MAX_PATH] = TEXT("<unknown>");
		PROCESS_MEMORY_COUNTERS 			pmc;
		UsageProc							*proc;
		
		m_mgr.ReportDebug("OpenProcess started");
	    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
	                                   PROCESS_VM_READ,
	                                   FALSE, pid );
	    if (NULL != hProcess ) {
	        HMODULE hMod;
	        DWORD cbNeeded;
			
			m_mgr.ReportDebug("OpenProcess succeed");
	        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				m_mgr.ReportDebug("EnumProcessModules succeed");
				itr = m_procs.find(pid);
				if (itr == m_procs.end()) {
					m_mgr.ReportDebug("Create new proc");
					proc = new UsageProc(pid);
					m_mgr.ReportDebug("Create new proc succeed");
					GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );
					proc->SetName(szProcessName);
					m_procs.insert(pair<DWORD, UsageProc *> (pid, proc));
					m_mgr.ReportDebug("Insert succeed");
				} else {
					proc = itr->second;
				}
				if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) ) {
					proc->SetMemUsage(pmc.WorkingSetSize / 1024);
				}
				proc->ExecGetUsage();
				proc->SetExists(true);
	        }
			CloseHandle( hProcess );
	    } else {
			m_mgr.ReportDebug("OpenProcess failed");
		}
	}
	m_mgr.ReportDebug("GetProcsData ended");
}

//
// report sending method
//
void		AgentCpu::SendStatusReport() {
	stringstream 		reportData;	
	CSystemCounters		data;
	
	m_pageColor = GREEN;
	ptime now = second_clock::local_time();
    date today = now.date();
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "] ";
	reportData << "up: " << (data.GetSystemUpTime() / 86400) << " days, ";
	reportData << data.GetServerSessions() << " users, ";
	reportData << countProcesses() << " procs, ";
	reportData << "load=" << (DWORD)m_systemWideCpuUsage << "%";
	reportData << "\n\n" << endl;
	if (m_psMode && m_firstPass == false) {
		procs_sorted_itr_t	itrSort;
		
		reportData << format("CPU    PID   %-16s  MemUsage") % "Image Name" << endl;
		for (itrSort = m_procsSorted.begin(); itrSort != m_procsSorted.end(); ++itrSort) {
			if ((*itrSort)->GetUsage() < 10)
				reportData << "0";
			reportData << format("%02.01f%%  %-4u  %-16s  %luk") % (*itrSort)->GetUsage() %  (*itrSort)->GetPid() % (*itrSort)->GetName() 
					% (*itrSort)->GetMemUsage() << endl;
		}
	}
	reportData << endl;
	if ((DWORD)m_systemWideCpuUsage >= m_warnPercent && (DWORD)m_systemWideCpuUsage < m_panicPercent) {
		if (m_curDelay >= m_delay)
			m_pageColor = YELLOW;
		++m_curDelay;
	} else if ((DWORD)m_systemWideCpuUsage >= m_panicPercent) {
		if (m_curDelay >= m_delay)
			m_pageColor = RED;
		++m_curDelay;
	} else { // reset delay
		m_curDelay = 0;
	}
	if (m_alwaysgreen)
		m_pageColor = GREEN;
	m_mgr.Status("cpu", bbcolors[m_pageColor], reportData.str());	
}

//
// Delete older procs
//
void		AgentCpu::DeleteOlderProcs() {	
	procs_itr				itr;
	list<DWORD>				toDelete;
	list<DWORD>::iterator	itrDel;
	
	for (itr = m_procs.begin(); itr != m_procs.end(); ++itr) {
		if (itr->second->GetExists() == false) {
			toDelete.push_back(itr->first);
		}
	}
	for (itrDel = toDelete.begin(); itrDel != toDelete.end(); ++itrDel) {
		delete m_procs[(*itrDel)];
		m_procs.erase((*itrDel));
	}
}

void		AgentCpu::InitProcs() {	
	procs_itr			itr;
	
	for (itr = m_procs.begin(); itr != m_procs.end(); ++itr) {
		itr->second->SetExists(false);
	}
}

void		AgentCpu::GetCpuData() {
	if (m_psMode) {
		GetProcsData();
	} else {
		m_systemWideCpuUsage = m_usage.GetCpuUsage();
	}
}

//
// Main running Method
//
void AgentCpu::Run() {
	procs_itr			itr;
	
	m_mgr.ReportDebug("cpu started");
	if (m_psMode)
		InitProcs();
	m_mgr.ReportDebug("InitProcs ended");
	GetCpuData();
	m_mgr.ReportDebug("GetCpuData ended");
	if (m_psMode) {
		DeleteOlderProcs();
		m_systemWideCpuUsage = 0.00;
		for (itr = m_procs.begin(); itr != m_procs.end(); ++itr) {
			m_procsSorted.insert(itr->second);
			m_systemWideCpuUsage += itr->second->GetUsage();
		}
	}
	m_mgr.ReportDebug("SendReport started");
	SendStatusReport();
	m_mgr.ReportDebug("SendReport ended");
	m_procsSorted.clear();
	m_firstPass = false;
	m_mgr.ReportDebug("cpu ended");
}

//
// destructor
//
AgentCpu::~AgentCpu() {
	procs_itr			itr;
	
	for (itr = m_procs.begin(); itr != m_procs.end(); ++itr) {
		delete (itr->second);
	}
	m_procs.clear();
}


bool AgentCpu::Init() {
	bbwinagentconfig_t		conf;
	
	m_mgr.ReportDebug("Init cpu started");
	if (m_mgr.LoadConfiguration(m_mgr.GetAgentName(), conf) == false) {
		m_mgr.ReportDebug("Init cpu failed");
		return false;
	}
	std::pair< bbwinagentconfig_iter_t, bbwinagentconfig_iter_t > 	range;
	range = conf.equal_range("setting");
	for ( ; range.first != range.second; ++range.first) {
		if (range.first->second["name"] == "alwaysgreen") {
			if (range.first->second["value"] == "true")
				m_alwaysgreen = true;
		}
		if (range.first->second["name"] == "psmode") {
			if (range.first->second["value"] == "false")
				m_psMode = false;
		}
		if (range.first->second["name"] == "default") {
			if (range.first->second["warnlevel"].size() > 0)
				m_warnPercent = m_mgr.GetNbr(range.first->second["warnlevel"]);
			if (range.first->second["paniclevel"].size() > 0)
				m_panicPercent = m_mgr.GetNbr(range.first->second["paniclevel"]);
			if (range.first->second["delay"].size() > 0)
				m_delay = m_mgr.GetNbr(range.first->second["delay"]);
		}
	}
	m_mgr.ReportDebug("Init cpu ended");
	return true;
}

AgentCpu::AgentCpu(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	m_procCount = CountProcessor();
	m_alwaysgreen = false;
	m_firstPass = true;
	m_psMode = true;
	m_warnPercent = DEF_CPU_WARN;
	m_panicPercent = DEF_CPU_PANIC;
	m_delay = DEF_CPU_DELAY;
	m_curDelay = 0;
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentCpu(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}

