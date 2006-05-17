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
#include <WtsApi32.h>

#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>

#include <string>

using namespace std;

#include "SystemCounters.h"


#define BBWIN_AGENT_EXPORTS

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"

using boost::format;
using namespace boost::posix_time;
using namespace boost::gregorian;

#include "ProcApi.h"

#include "CpuUsage.h"
#include "cpu.h"


static const char *bbcolors[] = { "green", "yellow", "red", NULL };


static const BBWinAgentInfo_t 		cpuAgentInfo =
{
	BBWIN_AGENT_VERSION,					// bbwinVersion;
	0,              						// agentMajVersion;
	4,              						// agentMinVersion;
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
	DWORD			count = 0;

	HINSTANCE hNtDll;
	NTSTATUS (WINAPI * pZwQuerySystemInformation)(UINT, PVOID, ULONG, PULONG);
	hNtDll = GetModuleHandle("ntdll.dll");
	assert(hNtDll != NULL);
	// find ZwQuerySystemInformation address
	*(FARPROC *)&pZwQuerySystemInformation =
		GetProcAddress(hNtDll, "ZwQuerySystemInformation");
	if (pZwQuerySystemInformation == NULL)
		return 0;
	HANDLE hHeap = GetProcessHeap();
	NTSTATUS Status;
	ULONG cbBuffer = 0x8000;
	PVOID pBuffer = NULL;
	// it is difficult to predict what buffer size will be
	// enough, so we start with 32K buffer and increase its
	// size as needed
	do
	{
		pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
		if (pBuffer == NULL)
			return 0;
		Status = pZwQuerySystemInformation(
			SystemProcessesAndThreadsInformation,
			pBuffer, cbBuffer, NULL);
		if (Status == STATUS_INFO_LENGTH_MISMATCH) {
			HeapFree(hHeap, 0, pBuffer);
			cbBuffer *= 2;
		}
		else if (!NT_SUCCESS(Status)) {
			HeapFree(hHeap, 0, pBuffer);
			return 0;
		}
	}
	while (Status == STATUS_INFO_LENGTH_MISMATCH);
	PSYSTEM_PROCESS_INFORMATION pProcesses = 
		(PSYSTEM_PROCESS_INFORMATION)pBuffer;
	for (;;) {
		++count;
		if (pProcesses->NextEntryDelta == 0)
			break ;
		pProcesses = (PSYSTEM_PROCESS_INFORMATION)(((LPBYTE)pProcesses)
			+ pProcesses->NextEntryDelta);
	}
	HeapFree(hHeap, 0, pBuffer);
	return count;
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
	m_mem = 0;
	m_priority = 0;
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

void 		AgentCpu::GetProcsOwners() {
	
	procs_itr			itr;
	PWTS_PROCESS_INFO	ppProcessInfom = NULL;
	DWORD				count;
	TCHAR				userbuf[ACCOUNT_SIZE];
	TCHAR				domainbuf[ACCOUNT_SIZE];
	DWORD				userSize, domainSize;
	SID_NAME_USE		type;
	
	if (m_useWts == false)
		return;
	count = 0;
	BOOL res = m_WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &ppProcessInfom, &count);
	if (res == 0) 
		return ;
	assert(ppProcessInfom != NULL);
	for (DWORD inc = 0; inc < count; ++inc) {
		userSize = ACCOUNT_SIZE;
		domainSize = ACCOUNT_SIZE;
		SecureZeroMemory(userbuf, ACCOUNT_SIZE);
		SecureZeroMemory(domainbuf, ACCOUNT_SIZE);
		BOOL res = LookupAccountSid(NULL, ppProcessInfom[inc].pUserSid, userbuf, &userSize,
			  domainbuf, &domainSize, &type);
		if (res) {
			stringstream 		username;	

			username << domainbuf << "\\" << userbuf;
			itr = m_procs.find(ppProcessInfom[inc].ProcessId);
			if (itr != m_procs.end()) {
				(*itr).second->SetOwner(username.str().c_str());
			}
		} else {
			itr = m_procs.find(ppProcessInfom[inc].ProcessId);
			if (itr != m_procs.end()) {
				(*itr).second->SetOwner("unknown");
			}
		}
	}
	m_WTSFreeMemory(ppProcessInfom);
}

void 		AgentCpu::GetProcsData() {
	string				procname;
	UsageProc			*proc;
	procs_itr			itr;

	m_mgr.ReportDebug("GetProcsData started");
	HINSTANCE hNtDll;
	NTSTATUS (WINAPI * pZwQuerySystemInformation)(UINT, PVOID, ULONG, PULONG);
	hNtDll = GetModuleHandle("ntdll.dll");
	assert(hNtDll != NULL);
	// find ZwQuerySystemInformation address
	*(FARPROC *)&pZwQuerySystemInformation =
		GetProcAddress(hNtDll, "ZwQuerySystemInformation");
	if (pZwQuerySystemInformation == NULL)
		return ;
	HANDLE hHeap = GetProcessHeap();
	NTSTATUS Status;
	ULONG cbBuffer = 0x8000;
	PVOID pBuffer = NULL;
	// it is difficult to predict what buffer size will be
	// enough, so we start with 32K buffer and increase its
	// size as needed
	do
	{
		pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
		if (pBuffer == NULL)
			return ;
		Status = pZwQuerySystemInformation(
			SystemProcessesAndThreadsInformation,
			pBuffer, cbBuffer, NULL);
		if (Status == STATUS_INFO_LENGTH_MISMATCH) {
			HeapFree(hHeap, 0, pBuffer);
			cbBuffer *= 2;
		}
		else if (!NT_SUCCESS(Status)) {
			HeapFree(hHeap, 0, pBuffer);
			return ;
		}
	}
	while (Status == STATUS_INFO_LENGTH_MISMATCH);
	PSYSTEM_PROCESS_INFORMATION pProcesses = 
		(PSYSTEM_PROCESS_INFORMATION)pBuffer;
	for (;;) {
		PCWSTR pszProcessName = pProcesses->ProcessName.Buffer;
		if (pszProcessName == NULL) {
			if (pProcesses->NextEntryDelta == 0)
				break ;
			pProcesses = (PSYSTEM_PROCESS_INFORMATION)(((LPBYTE)pProcesses)
			+ pProcesses->NextEntryDelta);
			continue ;
		}
		if (pProcesses->NextEntryDelta == 0)
			break ;
		CHAR szProcessName[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, pszProcessName, -1,
			szProcessName, MAX_PATH, NULL, NULL);
		procname = szProcessName;
		itr = m_procs.find(pProcesses->ProcessId);
		if (itr == m_procs.end()) {
			try {
				proc = new UsageProc(pProcesses->ProcessId);
			} catch (std::bad_alloc ex) {
				m_mgr.ReportInfo("Can't alloc memory");
				continue ;
			}
			if (proc == NULL) {
				m_mgr.ReportInfo("Can't alloc memory");
				continue ;
			}
			proc->SetName(procname);
			m_procs.insert(pair<DWORD, UsageProc *> (pProcesses->ProcessId, proc));
		} else {
			proc = itr->second;
		}
		proc->SetMemUsage(pProcesses->VmCounters.WorkingSetSize / 1024);
		proc->ExecGetUsage();
		proc->SetPriority(pProcesses->BasePriority);
		proc->SetExists(true);
		if (pProcesses->NextEntryDelta == 0)
			break ;
		pProcesses = (PSYSTEM_PROCESS_INFORMATION)(((LPBYTE)pProcesses)
			+ pProcesses->NextEntryDelta);
	}
	HeapFree(hHeap, 0, pBuffer);
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
		DWORD				count = 0;
		procs_sorted_itr_t	itrSort;
		
		if (m_limit != 0)
			reportData << "Information : ps mode is limited to " << m_limit << " lines\n" << endl;
		reportData << format("CPU    PID   %-16s  Pri") % "Image Name";
		if (m_useWts)
			reportData << format(" %-35s") % "Owner";
		reportData << " MemUsage" << endl;
		for (itrSort = m_procsSorted.begin(); itrSort != m_procsSorted.end(); ++itrSort) {
			if ((*itrSort)->GetUsage() < 10)
				reportData << "0";
			reportData << format("%02.01f%%  %-4u  %-16s  %-3u") % (*itrSort)->GetUsage() %  (*itrSort)->GetPid() % (*itrSort)->GetName() 
					% (*itrSort)->GetPriority();
			if (m_useWts)
				reportData << format(" %-35s") % (*itrSort)->GetOwner();
			reportData << format(" %luk") % (*itrSort)->GetMemUsage() << endl;
			if (m_limit != 0 && count > m_limit) {
				reportData << "..." << endl;
				break ;
			}
			++count;
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
	m_mgr.Status(m_testName.c_str(), bbcolors[m_pageColor], reportData.str().c_str());	
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
		GetProcsOwners();
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
	if (m_mWts) {
		FreeLibrary(m_mWts);
		m_mWts = NULL;
	}
}


bool AgentCpu::Init() {
	bbwinagentconfig_t		*conf;
	
	m_mgr.ReportDebug("Init cpu started");
	conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());
	if (conf == NULL) {
		m_mgr.ReportDebug("Init cpu failed");
		return false;
	}
	bbwinconfig_range_t * range = m_mgr.GetConfigurationRange(conf, "setting");
	if (range == NULL)
		return false;
	for ( ; range->first != range->second; ++range->first) {
		string		name, value;

		name = m_mgr.GetConfigurationRangeValue(range, "name");
		value = m_mgr.GetConfigurationRangeValue(range, "value");
		if (name == "alwaysgreen" && value == "true") {
			m_alwaysgreen = true;
		}
		if (name == "psmode" && value == "false") {
			m_psMode = false;
		}
		if (name == "testname" && value.length() > 0) {
			m_testName = value;
		}
		if (name == "limit" && value.length() >0) {
			m_limit = m_mgr.GetNbr(value.c_str());
		}
		if (name == "default") {
			string		warn, panic, delay;

			warn = m_mgr.GetConfigurationRangeValue(range, "warnlevel");
			panic = m_mgr.GetConfigurationRangeValue(range, "paniclevel");
			delay = m_mgr.GetConfigurationRangeValue(range, "delay");
			if (warn.size() > 0)
				m_warnPercent = m_mgr.GetNbr(warn.c_str());
			if (panic.size() > 0)
				m_panicPercent = m_mgr.GetNbr(panic.c_str());
			if (delay.size() > 0)
				m_delay = m_mgr.GetNbr(delay.c_str());
		}
	}
	m_mgr.FreeConfiguration(conf);
	m_mgr.ReportDebug("Init Wts Extension ended");
	InitWtsExtension();
	m_mgr.ReportDebug("Init cpu ended");
	return true;
}

void			AgentCpu::InitWtsExtension() {
	m_mWts = LoadLibrary("Wtsapi32.dll");
	if (m_mWts == NULL) {
		// no use of the wts extension
		return ;
	}
	m_WTSEnumerateProcesses = (WTSEnumerateProcesses_t)GetProcAddress(m_mWts, "WTSEnumerateProcessesA");
	m_WTSFreeMemory = (WTSFreeMemory_t)GetProcAddress(m_mWts, "WTSFreeMemory");
	if (m_WTSEnumerateProcesses && m_WTSFreeMemory)
		m_useWts = true;
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
	m_testName = "cpu";
	m_useWts = false;
	m_WTSEnumerateProcesses = NULL;
	m_WTSFreeMemory = NULL;
	m_limit = 0;
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentCpu(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
