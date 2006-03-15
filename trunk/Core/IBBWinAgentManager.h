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

#ifndef 	__IBBWINAGENTMANAGER_H__
#define		__IBBWINAGENTMANAGER_H__

#include <string>
#include <map>

#include "IBBWinException.h"

typedef std::map < std::string, std::string > 						bbwinagentconfig_attr_t;
typedef std::multimap < std::string, bbwinagentconfig_attr_t > 		bbwinagentconfig_t;
typedef bbwinagentconfig_t::iterator bbwinagentconfig_iter_t;

//
// BBWinAgentManager is from the simple design pattern facade 
// to be able to the agent to use bbwin callback
// this will allow agents to be written without re coding every thing
//
class   IBBWinAgentManager {
	public :
		// return the agent name
		virtual const std::string		& GetAgentName() = 0;
		// return agent file name (path)
		virtual const std::string		& GetAgentFileName() = 0;
		// return the agent timer setting
		virtual DWORD				GetAgentTimer() = 0;
		// get an environnement variable
		virtual void				GetEnvironmentVariable(const std::string & varname, std::string & dest) = 0;
		// get the last error in string str
		virtual void			GetLastErrorString(std::string & str) = 0; 
		// return a converted number
		virtual DWORD			GetNbr(const std::string & str ) = 0;
		// return number seconds from str example : "5m" returns 300
		virtual DWORD			GetSeconds(const std::string & str) = 0;
		// return a setting from the bbwin main configuration
		virtual const std::string		 & GetSetting(const std::string & settingName) = 0;
		
		//
		// stopping handles count : Only useful for agent using multiple threading or Wait functions
		//
		virtual void	GetHandles(HANDLE * hEvents) = 0;
		virtual DWORD	GetHandlesCount() = 0;
		
		// loading configuration functions on default bbwin configuration file bbwin.cfg : return true if success
		virtual bool	LoadConfiguration(const std::string & nameSpace, bbwinagentconfig_t & config) = 0;
		// loading configuration functions on custom configuration file 
		virtual bool	LoadConfiguration(const std::string & fileName, const std::string & nameSpace, bbwinagentconfig_t & config) = 0;
		
		// Report Functions : report in the bbwin log file
		virtual void 	ReportError(const std::string & str) = 0;
		virtual void 	ReportInfo(const std::string & str) = 0;
		virtual void 	ReportDebug(const std::string & str) = 0;
		virtual void 	ReportWarn(const std::string & str) = 0;
		
		// Event Report Functions : report in the Windows event log
		virtual void 	ReportEventError(const std::string & str) = 0;
		virtual void 	ReportEventInfo(const std::string & str) = 0;
		virtual void 	ReportEventWarn(const std::string & str) = 0;
		
		// hobbit protocol
		virtual void 	Status(const std::string & testName, const std::string & color, const std::string & text, const string & lifeTime = "") = 0;
		virtual void	Notify(const std::string & testName, const std::string & text) = 0;
		virtual void	Data(const std::string & dataName, const std::string & text) = 0;
		virtual void 	Disable(const std::string & testName, const std::string & duration, const std::string & text) = 0;
		virtual void	Enable(const std::string & testName) = 0;
		virtual void	Drop() = 0;
		virtual void	Drop(const std::string & testName) = 0;
		virtual void	Rename(const std::string & newHostName) = 0;
		virtual void	Rename(const std::string & oldTestName, const std::string & newTestName) = 0;
		virtual void	Message(const std::string & message, std::string & dest) = 0;
		virtual void	Config(const std::string & fileName, std::string & dest) = 0;
		virtual void	Query(const std::string & testName, std::string & dest) = 0;

		// virtual destructor 
		virtual ~IBBWinAgentManager() {};
};


#endif // !__IBBWINAGENTMANAGER_H__
