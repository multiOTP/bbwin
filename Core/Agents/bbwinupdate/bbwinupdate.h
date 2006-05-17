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

#ifndef		__BBWINUPDATE_H__
#define		__BBWINUPDATE_H__

#include <string>
#include "IBBWinAgent.h"

#define FILE_MAX_CONFIG_SIZE		32768
#define CONFIG_BUF_SIZE				1024

class AgentBBWinUpdate : public IBBWinAgent
{
	private :
		IBBWinAgentManager 		& m_mgr;
		std::string				m_fileName;

	public :
		AgentBBWinUpdate(IBBWinAgentManager & mgr);
		const BBWinAgentInfo_t & About();
		bool Init();
		void Run();
};


#endif 	// !__BBWINUPDATE_H__

