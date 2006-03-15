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

#include "disk.h"

static const BBWinAgentInfo_t 		diskAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	1,              					// agentMajVersion;
	0,              					// agentMinVersion;
	"disk",    					// agentName;
	"disk usage monitoring",        // agentDescription;
};                

const BBWinAgentInfo_t & AgentDisk::About() {
	return diskAgentInfo;
}

void 		AgentDisk::Run() {
	// stringstream 	reportData;	
        
	// reportData << "sample monitoring agent\n" << endl;
    // reportData << m_message << endl;
	// m_mgr.Status(m_testName, "green", reportData.str());
}

AgentDisk::AgentDisk(IBBWinAgentManager & mgr) : m_mgr(mgr) {
}

bool		AgentDisk::Init() {
	bbwinagentconfig_t		conf;
	
	if (m_mgr.LoadConfiguration(m_mgr.GetAgentName(), conf) == false)
		return false;
	std::pair< bbwinagentconfig_iter_t, bbwinagentconfig_iter_t > 	range;
	range = conf.equal_range("setting");
	for ( ; range.first != range.second; ++range.first) {
		// if (range.first->second["name"] == "testname") {
			// m_testName = range.first->second["value"];
		// }
		// if (range.first->second["name"] == "message") {
			// m_message = range.first->second["value"];
		// }
	}	
	return true;
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentDisk(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
