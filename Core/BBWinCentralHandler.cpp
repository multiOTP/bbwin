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
#include <assert.h>

#include <string>
#include <iostream>
#include <map>
using namespace std;

#include "ou_thread.h"
using namespace openutils;

#include "Logging.h"
#include "BBWinNet.h"
#include "BBWinHandler.h"
#include "BBWinCentralHandler.h"

#include "Utils.h"
using namespace utils;

//
//  FUNCTION: BBWinCentralHandler
//
//  PURPOSE: constructor
//
//  PARAMETERS:
//    
//    
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//
BBWinCentralHandler::BBWinCentralHandler(bbwinhandler_data_t & data) :
						m_timer (data.timer),
						m_hEvents (NULL),
						m_hCount (0)
{
	m_log = Logging::getInstancePtr();
	
	
}

//
//  FUNCTION: BBWinCentralHandler::run
//
//  PURPOSE: running part
//
//  PARAMETERS:
//    none
//    
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  thread run function. Thread is terminated at the end of this function
//  this function run each agent which can be managed with a central server
//
void BBWinCentralHandler::run() {
	DWORD 		dwWait;

	//IBBWinAgent	* agent = new HobbitClient();
	for (;;) {
		dwWait = WaitForMultipleObjects(m_hCount, m_hEvents , FALSE, m_timer * 1000 );
		if ( dwWait >= WAIT_OBJECT_0 && dwWait <= (WAIT_OBJECT_0 + m_hCount - 1) ) {    
			break ;
		} else if (dwWait >= WAIT_ABANDONED_0 && dwWait <= (WAIT_ABANDONED_0 + m_hCount - 1) ) {
			break ;
		}
	}
}


//
//  FUNCTION: BBWinCentralHandler::AddAgentHandler
//
//  PURPOSE: add an agent handler to the central mode manager
//
//  PARAMETERS:
//    handler		BBWinHandler corresponding to the agent instance
//    
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//
void	BBWinCentralHandler::AddAgentHandler(BBWinHandler *handler) {
	assert(handler != NULL);
	
	handler->SetCentralMode(true);
	//handler->SetClientDataCallBack();
	m_agents.push_back(handler);
}
