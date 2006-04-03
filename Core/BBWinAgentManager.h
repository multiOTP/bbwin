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

#ifndef 	__BBWINAGENTMANAGER_H__
#define		__BBWINAGENTMANAGER_H__

#include "IBBWinException.h"
#include "IBBWinAgentManager.h"

#include "Logging.h"
#include "BBWinHandlerData.h"
#include "BBWinNet.h"

class   BBWinAgentManager : public IBBWinAgentManager {
	private :
		std::string			& m_agentName;
		std::string			& m_agentFileName;
		HANDLE				*m_hEvents;
		DWORD				m_hCount;
		Logging				*m_log;
		bbdisplay_t			& m_bbdisplay;
		setting_t			& m_setting;
		DWORD				m_timer;
		BBWinNet			m_net;
		bool				m_logReportFailure;
		
	private :
		void	LoadFileConfiguration(const std::string & filePath, const std::string & nameSpace, bbwinagentconfig_t & config);
	
	public :
		BBWinAgentManager(const bbwinhandler_data_t & data);

		const std::string		&GetAgentName() { return m_agentName; };
		const std::string		&GetAgentFileName() { return m_agentFileName; };
		void			GetEnvironmentVariable(const std::string & varname, std::string & dest);
		void			GetLastErrorString(std::string & str);
		DWORD			GetNbr(const std::string & str );
		DWORD			GetSeconds(const std::string & str);
		const std::string &GetSetting(const std::string & settingName) { return m_setting[ settingName]; };
		
		DWORD	GetAgentTimer() { return m_timer; };
		void	GetHandles(HANDLE * hEvents);
		
		DWORD	GetHandlesCount() { return m_hCount; } ;
		
		bool	LoadConfiguration(const std::string & nameSpace, bbwinagentconfig_t & config);
		bool	LoadConfiguration(const std::string & fileName, const std::string & nameSpace, bbwinagentconfig_t & config);

		// hobbit protocol
		void 		Status(const std::string & testName, const std::string & color, const std::string & text, const string & lifeTime = "");
		void		Notify(const std::string & testName, const std::string & text);
		void		Data(const std::string & dataName, const std::string & text);
		void 		Disable(const std::string & testName, const std::string & duration, const std::string & text);
		void		Enable(const std::string & testName);
		void		Drop();
		void		Drop(const std::string & testName);
		void		Rename(const std::string & newHostName);
		void		Rename(const std::string & oldTestName, const std::string & newTestName);
		void		Message(const std::string & message, std::string & dest);
		void		Config(const std::string & fileName, std::string & dest);
		void		Query(const std::string & testName, std::string & dest);
		
		// Report Functions : report in the bbwin log file
		void 	ReportError(const std::string & str);
		void 	ReportInfo(const std::string & str);
		void 	ReportDebug(const std::string & str);
		void 	ReportWarn(const std::string & str);
		
		// Event Report Functions : report in the Windows event log
		void 	ReportEventError(const std::string & str);
		void 	ReportEventInfo(const std::string & str);
		void 	ReportEventWarn(const std::string & str);
};

#endif // __BBWINAGENTMANAGER_H__
