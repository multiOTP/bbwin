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

#ifndef __IBBWINAGENT_H__
#define __IBBWINAGENT_H__

#include "IBBWinAgentManager.h"


//
// agent version define : must be used in BBWinAgentInfo_s structure
//
#define			BBWIN_AGENT_VERSION			0x01

// 
// agent information structure
// used for future compatibility
//
typedef struct			BBWinAgentInfo_s
{
	DWORD				bbwinVersion;
	DWORD				agentMajVersion;
	DWORD				agentMinVersion;
	const char			*agentName;
	const char 			*agentDescription;
}						BBWinAgentInfo_t;

//
// Agent module class
//
class IBBWinAgent 
{
	public :
		virtual ~IBBWinAgent() {};
		// about function return a reference to your static BBWinAgentInfo_t 
		virtual const BBWinAgentInfo_t & About() = 0;
		// init function called only once after loaded the agent, return true on success, Agent will be unloaded if false returned
		virtual bool Init() = 0;
		// run method executed repetitively depending the timer setting
		virtual void Run() = 0;
};
	
#ifdef BBWIN_AGENT_EXPORTS
	#define BBWIN_AGENTDECL __declspec(dllexport)
#else
	#define BBWIN_AGENTDECL __declspec(dllimport)
#endif

typedef  IBBWinAgent  *(*CREATEBBWINAGENT)(IBBWinAgentManager & mgr);
typedef  void			(*DESTROYBBWINAGENT)(IBBWinAgent * agent);

extern "C" {
	BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr);
}

extern "C" {
	BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent);
}

#endif // !__IBBWINAGENT_H__

