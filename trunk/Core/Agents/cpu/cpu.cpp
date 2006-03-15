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

#include <pdh.h>
#include <Psapi.h>

#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"

using boost::format;

#include "CpuUsage.h"
#include "cpu.h"

const char				uptimeCounterPath[] = "\\\\.\\System\\System Up Time";
const char				sessionCounterPath[] = "\\\\.\\Server\\Server Sessions";


static const BBWinAgentInfo_t 		cpuAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	0,              // agentMajVersion;
	2,              // agentMinVersion;
	"cpu",    // agentName;
	"cpu agent : report cpu usage",        // agentDescription;
};                

const BBWinAgentInfo_t & AgentCpu::About() {
	return cpuAgentInfo;
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

static void PrintProcessNameAndID( DWORD processID )
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName, 
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }

    // Print the process name and identifier.
	cout << format("%s  (PID: %u)\n") % szProcessName % szProcessName << endl;

    CloseHandle( hProcess );
}



static void  countProcess() {
	 // Get the list of process identifiers.

    DWORD aProcesses[MAX_TABLE_PROC], cbNeeded, cProcesses;
    //unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

	  // for ( i = 0; i < cProcesses; i++ )
        //PrintProcessNameAndID( aProcesses[i] );

	
    // Print the name and process identifier for each process.
	std::cout <<  "Num process " << cProcesses << std::endl;
}


//
// return uptime in days
//
static 	DWORD			getUpDay() {
	DWORD		days;
	
	days = MyGetCounterValue <DWORD> (uptimeCounterPath) / 86400;
	return days;
}

#define		DIV 1024

static	void			getMemory(stringstream & data) {
	MEMORYSTATUS 			stat;
	size_t					totalPhys;
	size_t					usedPhys;
	
	GlobalMemoryStatus(&stat);
	totalPhys = stat.dwTotalPhys / DIV / DIV;
	usedPhys = totalPhys - (stat.dwAvailPhys / DIV / DIV);
	data << "PhysicalMem: " << totalPhys << "MB(";
	cout << totalPhys << " " << usedPhys << endl;
	data << (size_t)((100 * usedPhys) / totalPhys);
	data << "%)";
	data << endl << endl;
}

//
// return users sessions 
//
static 	DWORD			getSessions() {
	DWORD		sessions;
	
	sessions = MyGetCounterValue <DWORD> (sessionCounterPath);
	return sessions;
}

void AgentCpu::Run() {
	stringstream 	reportData;	

	int SystemWideCpuUsage = m_usage.GetCpuUsage();
	cout << "Cpu usage " << SystemWideCpuUsage << endl;
	
	using namespace boost::posix_time;
    using namespace boost::gregorian;

    //get the current time from the clock -- one second resolution
    ptime now = second_clock::local_time();
    //Get the date part out of the time
    date today = now.date();
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "] ";
	reportData << "up: " << getUpDay() << " days, ";
	reportData << getSessions() << " users, ";
	reportData << "procs, ";
	reportData << "load=" << SystemWideCpuUsage << "%, ";
	getMemory(reportData);
	reportData << endl;
    
	//countProcess();
	
	//cout << format("%1% %2% %3% %2% %1% \n") % "11" % "22" % "333"; // 'simple' style.
	
	cout << "Report :\n" << reportData.str() << endl;
	
	m_mgr.Status("cpu", "green", reportData.str());
}

bool AgentCpu::Init() {
	bbwinagentconfig_t		conf;
	
	if (m_mgr.LoadConfiguration(m_mgr.GetAgentName(), conf) == false)
		return false;
	std::pair< bbwinagentconfig_iter_t, bbwinagentconfig_iter_t > 	range;
	range = conf.equal_range("setting");
	for ( ; range.first != range.second; ++range.first) {
		// if (range.first->second["name"] == "alarmcolor") {
			// m_alarmColor = range.first->second["value"];
		// }
		// if (range.first->second["name"] == "counterpath") {
			// m_counterPath = range.first->second["value"];
		// }
	}
	m_usage.GetCpuUsage();
	return true;
}

AgentCpu::AgentCpu(IBBWinAgentManager & mgr) : m_mgr(mgr) {
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentCpu(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
