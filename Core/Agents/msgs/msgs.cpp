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

#include "msgs.h"

static const BBWinAgentInfo_t 		msgsAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	0,              // agentMajVersion;
	1,              // agentMinVersion;
	"msgs",    // agentName;
	"msgs agent : check the event viewer",        // agentDescription;
};                


//
// common bb colors
//
static const char	*bbcolors[] = {"green", "yellow", "red", NULL};

//
// Agent functions

const BBWinAgentInfo_t & AgentMsgs::About() {
	return msgsAgentInfo;
}


#define BUFFER_SIZE		1024

//
// Run method
// called from init
//
void AgentMsgs::Run() {
	stringstream 					reportData;	
	BBAlarmType						finalState;
	
    ptime now = second_clock::local_time();
	finalState = GREEN;
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "] ";
	reportData << endl;

	HANDLE h;
	DWORD cRecords; 
	DWORD dwOldest;

	// Open the System event log. 

	h = OpenEventLog(NULL,  // uses local computer 
		"System");     // source name 
	if (h == NULL) 
	{    
		printf("Could not open the System event log."); 
		return;
	}

	// Display the number of records and the oldest record. 
	if (GetNumberOfEventLogRecords(h, &cRecords)) 
		printf("There are %d records in the System log.\n", cRecords); 

	if (GetOldestEventLogRecord(h, &dwOldest))
		printf("The oldest record is: %d\n\n", dwOldest);

	CloseEventLog(h); 

	// Open the Application event log. 

	h = OpenEventLog(NULL,    // uses local computer  
		"Application");  // source name 
	if (h == NULL) 
	{
		printf("Could not open the Application event log."); 
		return;
	}

	// Display the number of records and the oldest record. 

	if (GetNumberOfEventLogRecords(h, &cRecords)) 
		printf("There are %d records in the Application log.\n", 
		cRecords); 

	if (GetOldestEventLogRecord(h, &dwOldest))
		printf("The oldest record is: %d\n", dwOldest);

	CloseEventLog(h); 



	EVENTLOGRECORD *pevlr; 
	BYTE bBuffer[BUFFER_SIZE]; 
	DWORD dwRead, dwNeeded, dwThisRecord; 

	// Open the Application event log. 

	h = OpenEventLog( NULL,    // use local computer
		"Application");   // source name
	if (h == NULL) 
	{
		printf("Could not open the Application event log."); 
		return;
	}

	pevlr = (EVENTLOGRECORD *) &bBuffer; 

	// Get the record number of the oldest event log record.

	GetOldestEventLogRecord(h, &dwThisRecord);

	// Opening the event log positions the file pointer for this 
	// handle at the beginning of the log. Read the event log records 
	// sequentially until the last record has been read. 

	while (ReadEventLog(h,                // event log handle 
		EVENTLOG_FORWARDS_READ |  // reads forward 
		EVENTLOG_SEQUENTIAL_READ, // sequential read 
		0,            // ignored for sequential reads 
		pevlr,        // pointer to buffer 
		BUFFER_SIZE,  // size of buffer 
		&dwRead,      // number of bytes read 
		&dwNeeded))   // bytes in next record 
	{
		while (dwRead > 0) 
		{ 
			// Print the record number, event identifier, type, 
			// and source name. 

			printf("%03d  Event ID: 0x%08X  Event type: ", 
				dwThisRecord++, pevlr->EventID); 

			switch(pevlr->EventType)
			{
			case EVENTLOG_ERROR_TYPE:
				printf("EVENTLOG_ERROR_TYPE\t  ");
				break;
			case EVENTLOG_WARNING_TYPE:
				printf("EVENTLOG_WARNING_TYPE\t  ");
				break;
			case EVENTLOG_INFORMATION_TYPE:
				printf("EVENTLOG_INFORMATION_TYPE  ");
				break;
			case EVENTLOG_AUDIT_SUCCESS:
				printf("EVENTLOG_AUDIT_SUCCESS\t  ");
				break;
			case EVENTLOG_AUDIT_FAILURE:
				printf("EVENTLOG_AUDIT_FAILURE\t  ");
				break;
			default:
				printf("Unknown ");
				break;
			}

			printf("Event source: %s\n", 
				(LPSTR) ((LPBYTE) pevlr + sizeof(EVENTLOGRECORD))); 
			
			printf("Description: %s\n", 
				(LPSTR) ((LPBYTE) pevlr + pevlr->StringOffset));

			dwRead -= pevlr->Length; 
			pevlr = (EVENTLOGRECORD *) 
				((LPBYTE) pevlr + pevlr->Length); 
		} 

		pevlr = (EVENTLOGRECORD *) &bBuffer; 
	} 

	CloseEventLog(h); 


	//m_mgr.Status("msgs", bbcolors[finalState], reportData.str().c_str());
}



//
// init function
//
bool AgentMsgs::Init() {
	bbwinagentconfig_t		*conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());
	
	if (conf == NULL)
		return false;
	bbwinconfig_range_t * range = m_mgr.GetConfigurationRange(conf, "setting");
	if (range == NULL)
		return false;
	for ( ; range->first != range->second; ++range->first) {
		
	}
	m_mgr.FreeConfigurationRange(range);
	m_mgr.FreeConfiguration(conf);
	return true;
}


//
// constructor 
//
AgentMsgs::AgentMsgs(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	
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
