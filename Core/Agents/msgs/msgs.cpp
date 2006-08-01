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

#include "msgs.h"

static const BBWinAgentInfo_t 		msgsAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	"msgs",								// agentName;
	"msgs agent : check the event logs",        // agentDescription;
	0									// flags
};                

using namespace EventLog;

//
// common bb colors
//
static const char	*bbcolors[] = {"green", "yellow", "red", NULL};



//
// AgentMsgs methods
//
//


//
// Run method
// called from init
//
void AgentMsgs::Run() {
	stringstream 					reportData;	
	DWORD							finalState;
	
    ptime now = second_clock::local_time();
	finalState = GREEN;
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "] ";
	reportData << endl;
	finalState = m_eventlog.Execute(reportData);
	if (m_alwaysgreen)
		finalState = GREEN;

	if (m_summary) {
		reportData << "\nSummary:\n" << endl;
		reportData << format("Events Analyzed: %6u") % m_eventlog.GetTotalCount() << endl ;
		reportData << format("Events Matched:  %6u") % m_eventlog.GetMatchCount() << endl ;
		reportData << format("Events Ignored:  %6u") % m_eventlog.GetIgnoreCount() << endl ;
		reportData << endl;
	}
	m_mgr.Status(m_testName.c_str(), bbcolors[finalState], reportData.str().c_str());
}

void	AgentMsgs::AddRule(PBBWINCONFIGRANGE range, bool ignore) {
	string		logfile, source, delay, type, alarmcolor, value, eventid;
	string		count, priority;
	EventLog::Rule		rule;
	
	logfile = m_mgr.GetConfigurationRangeValue(range, "logfile");
	if (logfile.length() == 0)
		return ;
	alarmcolor = m_mgr.GetConfigurationRangeValue(range, "alarmcolor");
	source = m_mgr.GetConfigurationRangeValue(range, "source");
	value = m_mgr.GetConfigurationRangeValue(range, "value");
	delay = m_mgr.GetConfigurationRangeValue(range, "delay");
	type = m_mgr.GetConfigurationRangeValue(range, "type");
	eventid = m_mgr.GetConfigurationRangeValue(range, "eventid");
	count = m_mgr.GetConfigurationRangeValue(range, "count");
	priority = m_mgr.GetConfigurationRangeValue(range, "priority");
	if (value.length() > 0)
		rule.SetValue(value);
	if (alarmcolor.length() > 0) {
		if (alarmcolor == "green") 
			rule.SetAlarmColor(GREEN);
		else if (alarmcolor == "yellow")
			rule.SetAlarmColor(YELLOW);
		else if (alarmcolor == "red")
			rule.SetAlarmColor(RED);
	}
	if (source.length() > 0)
		rule.SetSource(source);
	rule.SetDelay(m_delay);
	if (delay.length() > 0) {
		DWORD del = m_mgr.GetSeconds(delay.c_str());
		if (del >= 60)
			rule.SetDelay(del);
	}
	if (eventid.length() > 0)
		rule.SetEventId(m_mgr.GetNbr(eventid.c_str()));
	if (priority.length() > 0)
		rule.SetPriority(m_mgr.GetNbr(priority.c_str()));
	if (count.length() > 0)
		rule.SetCount(m_mgr.GetNbr(count.c_str()));
	if (type.length() > 0)
		rule.SetType(type);
	rule.SetIgnore(ignore);
	m_eventlog.AddRule(logfile, rule);
}

//
// init function
//
bool AgentMsgs::Init() {
	PBBWINCONFIG		conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());
	
	if (conf == NULL)
		return false;
	m_mgr.ReportDebug("Begin Msgs Initialization");
	PBBWINCONFIGRANGE range = m_mgr.GetConfigurationRange(conf, "setting");
	if (range == NULL)
		return false;
	for ( ; m_mgr.AtEndConfigurationRange(range); m_mgr.IterateConfigurationRange(range)) {
		string	name =  m_mgr.GetConfigurationRangeValue(range, "name");
		string  value = m_mgr.GetConfigurationRangeValue(range, "value");
		if (name == "alwaysgreen" && value == "true")
			m_alwaysgreen = true;
		if (name == "summary" && value == "true")
			m_summary = true;
		if (name == "testname" && value.length() > 0)
			m_testName = value;
		if (name == "delay" && value.length() > 0) {
			DWORD del = m_mgr.GetSeconds(value.c_str());
			if (del >= 60)
				m_delay = del;
		}
	}
	m_mgr.FreeConfigurationRange(range);
	range = m_mgr.GetConfigurationRange(conf, "match");
	if (range == NULL)
		return false;
	for ( ; m_mgr.AtEndConfigurationRange(range); m_mgr.IterateConfigurationRange(range)) {
		AddRule(range, false);
	}
	m_mgr.FreeConfigurationRange(range);
	range = m_mgr.GetConfigurationRange(conf, "ignore");
	if (range == NULL)
		return false;
	for ( ; m_mgr.AtEndConfigurationRange(range); m_mgr.IterateConfigurationRange(range)) {
		AddRule(range, true);
	}
	m_mgr.FreeConfigurationRange(range);
	m_mgr.FreeConfiguration(conf);
	m_mgr.ReportDebug("Ending Msgs Initialization");
	return true;
}


//
// constructor 
//
AgentMsgs::AgentMsgs(IBBWinAgentManager & mgr) : 
		m_mgr(mgr),
		m_delay(30 * 60),
		m_alwaysgreen(false),
		m_summary(false),
		m_testName("msgs")
{

}


//
// Destructor 
//
AgentMsgs::~AgentMsgs() {
	
}


//
// common agents export functions
//

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentMsgs(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}

BBWIN_AGENTDECL const BBWinAgentInfo_t * GetBBWinAgentInfo() {
	return &msgsAgentInfo;
}
