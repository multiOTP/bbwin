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

#ifndef		__CPU_H__
#define		__CPU_H__

#include <string>

#include "IBBWinAgent.h"

#define	 CPU_DELAY		0


#define	MAX_TABLE_PROC	1024

struct proc_s {
	CCpuUsage 		usage;
	std::string		name;
};

typedef struct proc_s		proc_t;

class AgentCpu : public IBBWinAgent
{
	private :
		IBBWinAgentManager 		& m_mgr;
		CCpuUsage 				m_usage;
		
	public :
		AgentCpu(IBBWinAgentManager & mgr);
		const BBWinAgentInfo_t & About();
		bool Init();
		void Run();
};


#endif 	// !__CPU_H__

