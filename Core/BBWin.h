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

#include <map>
#include <string>
using namespace std;

#ifndef 	__BBWIN_H__
#define		__BBWIN_H__

#define BB_PATH_LEN		4096

#include "IBBWinException.h"

#include "Singleton.h"

using namespace DesignPattern;

#define	BBWIN_CONFIG_FILENAME		"bbwin.cfg"
#define BBWIN_NAMESPACE_CONFIG		"bbwin"

#define BBWIN_LOG_DEFAULT_PATH		"C:\\BBWin.log"

#define	BBWIN_STOP_HANDLE			0
#define BBWIN_AUTORELOAD_HANDLE		1
#define BBWIN_MAX_COUNT_HANDLE		2

#define BBWIN_AUTORELOAD_INTERVAL	5

#include "Logging.h"
#include "BBWinHandler.h"
#include "BBWinConfig.h"

class BBWin;

// pointer treatment function 
typedef void (BBWin::*load_simple_setting)(const std::string & name, const std::string & value);


class BBWin : public Singleton< BBWin > {
	private :
		// Configuration and Logs
		Logging					*m_log;
		bbwinconfig_t			m_configuration;
		BBWinConfig				*m_conf;
		
		// BBWin Handle 
		HANDLE					m_hEvents[BBWIN_MAX_COUNT_HANDLE];
		DWORD					m_hCount;

		// Autoreload feature
		FILETIME				m_confTime;
		bool					m_autoReload;
		DWORD					m_autoReloadInterval;

		//
		// STL Containers
		//
		std::map< std::string, BBWinHandler * >			m_handler; // handle the agents instances
		std::vector< std::string >						m_bbdisplay; // handle the bbdisplay list
		std::vector< std::string >						m_bbpager; // handle the bbpager list
		std::map< std::string, std::string >			m_setting; // handle the general settings
		std::map< std::string, load_simple_setting > 	m_defaultSettings; // handle the default settings ma
		
	private :
		void			BBWinRegQueryValueEx(HKEY hKey, const std::string & key, std::string & dest);
		
		void 			ReportEvent(DWORD dwEventID, WORD cInserts, LPCTSTR *szMsg);
		void			Init(HANDLE h);
		void			InitSettings();
		void			WaitFor();
		
		void			AddSimpleSetting(const std::string & name, const std::string & value);
		void			AddBBDisplay(const std::string & name, const std::string & value);
		void			AddBBPager(const std::string & name, const std::string & value);

		void			SetAutoReload(const std::string & name, const std::string & value);
		bool			GetConfFileChanged();
		
		void 			LoadRegistryConfiguration();
		void			LoadConfiguration();
		void			CheckConfiguration();
		void			LoadAgents();
		void			UnloadAgents();
		
	public :
		BBWin();
		~BBWin();
		void Start(HANDLE h);
		void Reload();
		void Stop();
};


/** class BBWinException 
*/
class BBWinException : public IBBWinException {
public:
	BBWinException(const char* m);
	string getMessage() const;
};	


#endif // !__BBWIN_H__
