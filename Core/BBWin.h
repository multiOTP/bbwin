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

#define BBWIN_MAX_COUNT_HANDLE		1

#include "Logging.h"
#include "BBWinHandler.h"
#include "BBWinConfig.h"

class BBWin;

// pointer treatment function 
typedef void (BBWin::*load_simple_setting)(const std::string & name, const std::string & value);


class BBWin : public Singleton< BBWin > {
	private :
		std::string 			m_tmpPath;
		std::string				m_etcPath;
		std::string				m_binPath;
		std::string				m_bbwinConfigPath;
		HANDLE					m_hEvents[BBWIN_MAX_COUNT_HANDLE];
		DWORD					m_hCount;
		Logging					*m_log;
		bbwinconfig_t			m_configuration;
		BBWinConfig				*m_conf;
		
		std::map< std::string, BBWinHandler * >			m_handler;
		std::vector< std::string >						m_bbdisplay;
		std::map< std::string, std::string >			m_setting;
		std::map< std::string, load_simple_setting > 	m_defaultSettings;
		
	private :
		void				BBWinRegQueryValueEx(HKEY hKey, const std::string & key, std::string & dest);
		
		void 				ReportEvent(DWORD dwEventID, WORD cInserts, LPCTSTR *szMsg);
		void				Init(HANDLE h);
		void				WaitFor();
		
		void				addSimpleSetting(const std::string & name, const std::string & value);
		void				addBBDisplay(const std::string & name, const std::string & value);
		
		void 			LoadRegistryConfiguration();
		void			LoadConfiguration();
		void			CheckConfiguration();
		void			LoadAgents();
		void			UnloadAgents();
		
	public :
		BBWin();
		~BBWin();
		void Start(HANDLE h);
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
