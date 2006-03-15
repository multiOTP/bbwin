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

#include <Psapi.h>

#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"

using namespace boost::posix_time;
using namespace boost::gregorian;

#include "procs.h"

static const BBWinAgentInfo_t 		procsAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	0,              // agentMajVersion;
	2,              // agentMinVersion;
	"procs",    // agentName;
	"procs agent : check running processes",        // agentDescription;
};                


//
// Simple check rules functions
//
static bool 	EqualProcRule(DWORD cur, DWORD rule) {
	return (cur == rule);
}

static bool 	SupProcRule(DWORD cur, DWORD rule) {
	return (cur > rule);
}

static bool 	SupEqualProcRule(DWORD cur, DWORD rule) {
	return (cur >= rule);
}

static bool 	InfProcRule(DWORD cur, DWORD rule) {
	return (cur < rule);
}

static bool 	InfEqualProcRule(DWORD cur, DWORD rule) {
	return (cur <= rule);
}


//
// common bb colors
//
static const char	*bbcolors[] = {"green", "yellow", "red", NULL};


//
// global rules used for procs
//
const static Rule_t			globalRules[]  = 
{
	{"<=", InfEqualProcRule},
	{">=", SupEqualProcRule},
	{"==", EqualProcRule},
	{"<", InfProcRule},
	{"=", EqualProcRule},
	{">", SupProcRule},
	{NULL, NULL},
};

//
// check if process is running
// it will return the number of instances
// 
static DWORD 		CountProcesses(const DWORD *aProc, const DWORD cProc, const string & name) {
    TCHAR 			szProcessName[MAX_PATH] = TEXT("<unknown>");
	DWORD			count = 0;
	
	for (DWORD i = 0; i < cProc; ++i) {
	    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
	                                   PROCESS_VM_READ,
	                                   FALSE, aProc[i]);
	    if (NULL != hProcess ) {
	        HMODULE hMod;
	        DWORD cbNeeded;

	        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
	             &cbNeeded) )
	        {
				GetModuleBaseName( hProcess, hMod, szProcessName, 
	                               sizeof(szProcessName)/sizeof(TCHAR) );
				string 	curName = szProcessName;
				std::transform( curName.begin(), curName.end(), curName.begin(), tolower ); 
				size_t	res = curName.find(name);
				if (res >= 0 && res < curName.size()) {
					++count;
				}
			}
	    }
	    CloseHandle( hProcess );
	}
	return count;
}



//
// Agent functions

const BBWinAgentInfo_t & AgentProcs::About() {
	return procsAgentInfo;
}


//
// check processes from the rules configured
// it will return the bbcolor final state
// 0 green
// 1 yellow
// 2 red
// 
BBAlarmType				AgentProcs::ExecProcRules(stringstream & reportData) {
    DWORD 							aProcesses[MAX_TABLE_PROC], cbNeeded, cProcesses;
	list<ProcRule_t *>::iterator 	itr;
	stringstream					report;
	BBAlarmType						state;

	state = GREEN;
    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return state;
    cProcesses = cbNeeded / sizeof(DWORD);
	for (itr = m_rules.begin(); itr != m_rules.end(); ++itr) {
		DWORD count = CountProcesses(aProcesses, cProcesses, (*itr)->name);
		if ((*itr)->apply_rule(count, (*itr)->count)) {
			report << "&green " << (*itr)->name << " " << (*itr)->rule << " - " << count << " instance";
			if (count > 1)
				report << "s";
			report << " running" << endl;
		} else {
			report << "&" << bbcolors[(*itr)->color] << " " << (*itr)->name << " " << (*itr)->rule << " - " << count << " instance";
			if (count > 0)
				report << "s";
			report << " running" << endl;
			if ((*itr)->color > state)
				state = (*itr)->color;
		}
	}
	if (state == GREEN)
		reportData << "All processes are ok\n" << report.str() << endl;
	else
		reportData << "Some processes are in error\n" << endl << report.str() << endl;
	return state;
}


//
// Run method
// called from init
//
void AgentProcs::Run() {
	stringstream 					reportData;	
	BBAlarmType						finalState;
	
    ptime now = second_clock::local_time();
	finalState = GREEN;
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "] ";
	if (m_rules.size() == 0)
		reportData << "No process to check" << endl;
	else
		finalState = ExecProcRules(reportData);
	reportData << endl;
	m_mgr.Status("procs", bbcolors[finalState], reportData.str());
}

//
// Add rule method
// called from init
//
void AgentProcs::AddRule(const string & name, const string & rule, const string & color) {
	if (name.length() == 0 || rule.length() == 0) {
		m_mgr.ReportEventWarn("some rules are incorrect. Please check your configuration.");
		return ;
	}
	ProcRule_t		*procRule;
	procRule = new ProcRule_t;
	procRule->name = name;
	procRule->color = YELLOW;
	if (color.length() > 0) {
		if (color == "red")
			procRule->color = RED;
	}
	procRule->rule = rule;
	procRule->apply_rule = NULL;
	procRule->count = 0;
	for (int i = 0; globalRules[i].apply_rule != NULL; ++i) {
		size_t	res = rule.find(globalRules[i].name);
		if (res >= 0 && res < rule.size()) {
			procRule->apply_rule = globalRules[i].apply_rule;
			procRule->count = m_mgr.GetNbr(rule.substr(strlen(globalRules[i].name), rule.length()));
			break ;
		}
	}
	if (procRule->apply_rule == NULL) {
		m_mgr.ReportEventWarn("some rules are incorrect. Please check your configuration.");
		delete procRule;
	} else {
		m_rules.push_back(procRule);
	}
}


//
// init function
//
bool AgentProcs::Init() {
	bbwinagentconfig_t		conf;
	
	if (m_mgr.LoadConfiguration(m_mgr.GetAgentName(), conf) == false)
		return false;
	std::pair< bbwinagentconfig_iter_t, bbwinagentconfig_iter_t > 	range;
	range = conf.equal_range("setting");
	for ( ; range.first != range.second; ++range.first) {
		AddRule(range.first->second["name"], range.first->second["rule"], range.first->second["alarmcolor"]);
	}
	return true;
}


//
// cosntructor 
//
AgentProcs::AgentProcs(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	
}

//
// Destructor 
//
AgentProcs::~AgentProcs() {
	std::list<ProcRule_t *>::iterator 	itr;
	
	for (itr = m_rules.begin(); itr != m_rules.end(); ++itr) {
		delete (*itr);
	}
	m_rules.clear();
}


//
// common agents export functions
//

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentProcs(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
