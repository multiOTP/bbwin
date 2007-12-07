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

using boost::format;
using namespace boost::posix_time;
using namespace boost::gregorian;


#include "Stats.h"


static const BBWinAgentInfo_t 		statsAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	"stats",							// agentName;
	"stats agent :  report general stats for trends purpose",        // agentDescription;
	BBWIN_AGENT_CENTRALIZED_COMPATIBLE	// flags
};                

#define		TEMP_PATH_LEN		1024


static	int			MyNetstatExec(const string & cmd, const string & path) {
	string			execCmd;

	execCmd = cmd + " > " + path;
	return system(execCmd.c_str());
}

void	AgentStats::NetstatLocal(const string & path) {
	stringstream 	reportData;	

	ptime now = second_clock::local_time();
	date today = now.date();
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "]";
	reportData << endl;
	MyNetstatExec((string)"netstat -s", path);
	ifstream netstatFile(path.c_str());
	if (netstatFile) {
		string 		line;
		size_t		ret;
		string 		prefix;

		reportData << "win32" << endl;	
		while (getline(netstatFile, line)) {
			if ((ret = line.find("IP Statistics")) >= 0 && ret < line.size()) {
				prefix = "ip";
				reportData << endl;
			} else if ((ret = line.find("ICMP Statistics")) >= 0 && ret < line.size()) {
				prefix = "icmp";
				reportData << endl;
			} else if ((ret = line.find("TCP Statistics")) >= 0 && ret < line.size()) {
				prefix = "tcp";
				reportData << endl;
			} else if ((ret = line.find("UDP Statistics")) >= 0 && ret < line.size()) {
				prefix = "udp";
				reportData << endl;
			} else if ((ret = line.find("=")) >= 0 && ret < line.size()) {
				reportData << prefix;
				for (size_t i = 0; i < line.size(); ++i) {
					if (line[i] != ' ' && line[i] != '\t')
						reportData << line[i];
				}
				reportData << endl;
			}
		}
	}
	reportData << endl;
	m_mgr.Status(m_testName.c_str(), "green", reportData.str().c_str());
}

void	AgentStats::NetstatCentralizedStep(const string & cmd, const string & path, const string dataName) {
	stringstream 	reportData;	

	if (MyNetstatExec(cmd, path)) {
		string err = (string)"failed to execute " + cmd;
		m_mgr.Log(LOGLEVEL_ERROR,err.c_str());
		m_mgr.ReportEventError(err.c_str());
	}
	ifstream fileOut(path.c_str());
	if (fileOut) {
		string 		line;
		string 		prefix;

		while (getline(fileOut, line)) {
			size_t i;

			// trunk space
			for (i = 0; i < line.size() && (line[i] == ' ' || line[i] == '\t'); ++i)
				;
			if (i == (line.size() - 1))
				continue ;
			reportData << line.substr(i) << endl;
		}
	} else {
		string err = (string)"failed to open report file " + path;
		m_mgr.Log(LOGLEVEL_ERROR,err.c_str());
		m_mgr.ReportEventError(err.c_str());
	}
	m_mgr.ClientData(dataName.c_str(), reportData.str().c_str());
}


void	AgentStats::NetstatCentralized(const string & path) {
	NetstatCentralizedStep("netstat -s", path, "netstat");
	NetstatCentralizedStep("netstat -na", path, "ports");
	NetstatCentralizedStep("netstat -nr", path, "route");
	NetstatCentralizedStep("ipconfig /all", path, "ipconfig");
}


void AgentStats::Run() {
	string		path;
	TCHAR			buf[TEMP_PATH_LEN + 1];

	m_mgr.GetEnvironmentVariable("TEMP", buf, TEMP_PATH_LEN);
	path = buf;
	path += "\\netstat.tmp";
	if (m_mgr.IsCentralModeEnabled() == false) {
		NetstatLocal(path);
	} else {
		NetstatCentralized(path);
	}
}

bool AgentStats::Init() {
	if (m_mgr.IsCentralModeEnabled() == false) {
		PBBWINCONFIG		conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());

		if (conf == NULL)
			return false;
		PBBWINCONFIGRANGE range = m_mgr.GetConfigurationRange(conf, "setting");
		if (range == NULL)
			return false;
		for ( ; m_mgr.AtEndConfigurationRange(range); m_mgr.IterateConfigurationRange(range)) {
			string name, value;

			name = m_mgr.GetConfigurationRangeValue(range, "name");
			value = m_mgr.GetConfigurationRangeValue(range, "value");
			if (name == "testname") {
				m_testName = value;
			}
		}	
		m_mgr.FreeConfigurationRange(range);
		m_mgr.FreeConfiguration(conf);
	}
	return true;
}

AgentStats::AgentStats(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	m_testName = "netstat";
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentStats(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}

BBWIN_AGENTDECL const BBWinAgentInfo_t * GetBBWinAgentInfo() {
	return &statsAgentInfo;
}
