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
#include <sddl.h>
#include <process.h>
#include <tchar.h>
#include <iostream>

#include <map>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

#include "ou_thread.h"
using namespace openutils;

#include "Logging.h"
#include "BBWinConfig.h"
#include "BBWinService.h"
#include "BBWin.h"
#include "BBWinAgentManager.h"
#include "IBBWinAgent.h"
#include "BBWinHandler.h"
#include "BBWinMessages.h"

#include "Utils.h"
using namespace utils;

// this event is signalled when the
// service should end
//
HANDLE  hServiceStopEvent = NULL;
BBWin	* bbw = NULL;

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service that does the work.
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
VOID ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv) {
	// report the status to the service control manager.
	//
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000)) 
      return ;

	// create the event object. The control handler function signals
	// this event when it receives the "stop" control code.
	//
	hServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if ( hServiceStopEvent == NULL)
      return ;
	// report the status to the service control manager.
	//
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
	{
		CloseHandle(hServiceStopEvent);
		return ;
	}

	bbw = BBWin::getInstancePtr();

	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
	{
		CloseHandle(hServiceStopEvent);
		return ;
	}
	if (!ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0))
    {
		CloseHandle(hServiceStopEvent);
		return ;
	}
	//
	// End of initialization
	//
	
	try {
		bbw->Start(hServiceStopEvent);
	} catch (BBWinException ex) {
		cout << "can't start : " << ex.getMessage();
		CloseHandle(hServiceStopEvent);
		hServiceStopEvent = NULL;
	}

	ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 3000);
	bbw->Stop();
	BBWin::freeInstance();
	
	if (hServiceStopEvent)
		CloseHandle(hServiceStopEvent);
}


//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//
VOID ServiceStop() {
	if ( hServiceStopEvent )
		SetEvent(hServiceStopEvent);
	
}


//
//  FUNCTION: BBWin
//
//  PURPOSE: BBWin class constructor 
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// Instantiate the logging class and set the default log path file
//
BBWin::BBWin() {
	m_log = Logging::getInstancePtr();
	m_conf = BBWinConfig::getInstancePtr();
	m_log->setFileName(BBWIN_LOG_DEFAULT_PATH);
	
	// set default setting known by BBWin
	m_defaultSettings[ "bbdisplay" ] = &BBWin::addBBDisplay;
	m_defaultSettings[ "hostname" ] = &BBWin::addSimpleSetting;
	m_defaultSettings[ "timer" ] = &BBWin::addSimpleSetting;
	m_defaultSettings[ "loglevel" ] = &BBWin::addSimpleSetting;
	m_defaultSettings[ "logpath" ] = &BBWin::addSimpleSetting;
	m_defaultSettings[ "logreportfailure" ] = &BBWin::addSimpleSetting;
	m_setting[ "timer" ] = "300";
	m_setting[ "loglevel" ] = "0";
	m_setting[ "logpath" ] = BBWIN_LOG_DEFAULT_PATH;
	m_setting[ "bbwinconfigfilename" ] = BBWIN_CONFIG_FILENAME;
}


//
//  FUNCTION: ~BBWin
//
//  PURPOSE: BBWin class destructor 
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// Free the logging class instance
//
BBWin::~BBWin() {
	BBWinConfig::freeInstance();
	Logging::freeInstance();
}


//
//  FUNCTION: BBWin::BBWinRegQueryValueEx
//
//  PURPOSE: get a registry value
//
//  PARAMETERS:
//     hKey : handle to the registry
//     key : key name
//     dest : key value destination 
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// Read a registry value
//
void				BBWin::BBWinRegQueryValueEx(HKEY hKey, const string & key, string & dest) {
	unsigned long 	lDataType;
	unsigned long 	lBufferLength = BB_PATH_LEN;
	char	 		tmpPath[BB_PATH_LEN + 1];
	string			err;

	if (RegQueryValueEx(hKey, key.c_str(), NULL, &lDataType, (LPBYTE)tmpPath, &lBufferLength) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		err = key;
		err += " : can't open it";
		throw BBWinException(err.c_str());
	}
	if (lDataType != REG_SZ)
	{
		RegCloseKey(hKey);
		err = key;
		err += ": invalid type";
		throw BBWinException(err.c_str());
	}
	if (lBufferLength == BB_PATH_LEN)
		tmpPath[BB_PATH_LEN] = '\0';
	dest = tmpPath;
}

//
//  FUNCTION: BBWin::LoadRegistryConfiguration
//
//  PURPOSE: load the registry configuration
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// Read the BBWin registry configuration
//
void 				BBWin::LoadRegistryConfiguration() {
	HKEY 			hKey;
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\BBWin"), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{		
		LPCTSTR		arg[] = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\BBWin : can't open it", NULL};
		
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_NO_REGISTRY, 1, arg);
		throw BBWinException("Can't open HKEY_LOCAL_MACHINE\\SOFTWARE\\BBWin");
	}
	try {
		BBWinRegQueryValueEx(hKey, "tmpPath", m_tmpPath);
		BBWinRegQueryValueEx(hKey, "binPath", m_binPath);
		BBWinRegQueryValueEx(hKey, "etcPath", m_etcPath);
	} catch (BBWinException ex) {
		LPCTSTR		arg[] = {ex.getMessage().c_str(), NULL};
		
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_NO_REGISTRY, 1, arg);
		throw ex;
	}
	RegCloseKey(hKey);
}

//
//  FUNCTION: BBWin::Init
//
//  PURPOSE: init the BBWin engine
//
//  PARAMETERS:
//    h  	service stop event handle
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// Init the BBWin engine by setting the handle table and 
// call the registry configuration
//
void			BBWin::Init(HANDLE h) {
	m_hEvents[0] = h;
	m_hCount = BBWIN_MAX_COUNT_HANDLE;
	try {
		LoadRegistryConfiguration();
	} catch (BBWinException ex) {
		throw ex;
	}
}

//
//  FUNCTION: BBWin::addSimpleSetting
//
//  PURPOSE: add a simple setting to the global setting map
//
//  PARAMETERS:
//    name 		setting name
//    value		value
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void			BBWin::addSimpleSetting(const std::string & name, const std::string & value) {
	//m_setting.insert(pair < string, string > (name, value));
	m_setting[name] = value;
}

//
//  FUNCTION: BBWin::addBBDisplay
//
//  PURPOSE:  add a BBDisplay to the BBDisplay vector
//
//  PARAMETERS:
//    name 		setting name
//    value		value
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//
void			BBWin::addBBDisplay(const string & name, const string & value) {
	m_bbdisplay.push_back(value);
}



//
//  FUNCTION: BBWin::LoadConfiguration
//
//  PURPOSE: load the configuration from file
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// Entry point to Load the configuration file
//
void							BBWin::LoadConfiguration() {
	m_bbwinConfigPath = m_etcPath;
	m_bbwinConfigPath += "\\";
	m_bbwinConfigPath += BBWIN_CONFIG_FILENAME;
	try {	
		m_conf->LoadConfiguration(m_bbwinConfigPath, BBWIN_NAMESPACE_CONFIG, m_configuration);
	} catch (BBWinConfigException ex) {
		string		err = ex.getMessage();
		LPCTSTR		arg[] = {m_bbwinConfigPath.c_str(), err.c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_NO_CONFIGURATION, 2, arg);
		throw ex;
	}
}

//
//  FUNCTION: BBWin::CheckConfiguration
//
//  PURPOSE: check the configuration loaded
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// Check if all necessary settings are present
//
void				BBWin::CheckConfiguration() {
	std::pair< config_iter_t, config_iter_t > range;
	range = m_configuration.equal_range("setting");
	for ( ; range.first != range.second; ++range.first) {
		map< string, load_simple_setting >::const_iterator i = m_defaultSettings.find( range.first->second["name"] );
		if ( i == m_defaultSettings.end() ) {
			LPCTSTR		arg[] = {BBWIN_CONFIG_FILENAME, BBWIN_NAMESPACE_CONFIG, "setting", range.first->second["name"].c_str(), NULL};
			m_log->reportWarnEvent(BBWIN_SERVICE, EVENT_INVALID_CONFIGURATION, 4, arg);
		} else {
			// experimenting pointer to member functions :) some people say "it's nice", some people say "it's ugly"
			(this->*(i->second))(range.first->second["name"], range.first->second["value"]);
		}
	}
	if (m_bbdisplay.size() == 0) {
		LPCTSTR		arg[] = {m_bbwinConfigPath.c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_NO_BB_DISPLAY, 1, arg);
		throw BBWinConfigException("no bbdisplay configured");
	}
	if (m_setting["hostname"].size() == 0) {
		LPCTSTR		arg[] = {m_bbwinConfigPath.c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_NO_HOSTNAME, 1, arg);
		throw BBWinConfigException("no hostname configured");
	}
	if (m_setting[ "logpath" ].size() != 0) {
		m_log->setFileName(m_setting[ "logpath" ]);
	}
	if (m_setting[ "loglevel" ].size() != 0 ) {
		m_log->setLogLevel(GetNbr(m_setting[ "loglevel" ]));
	}
	m_setting[ "tmppath" ] = m_tmpPath;
	m_setting[ "etcpath" ] = m_etcPath;
	m_setting[ "binpath" ] = m_binPath;
	if (SetCurrentDirectory(m_setting[ "binpath" ].c_str()) == 0) {
		LPCTSTR		arg[] = {m_setting[ "binpath" ].c_str(), NULL};
		m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_BAD_PATH, 1, arg);
		throw BBWinConfigException("bad BBWin bin path");
	}
}

	
//
//  FUNCTION: BBWin::LoadAgents
//
//  PURPOSE: load the agents
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  very important function. It starts the agent threads.
// agent timer can be personnalized for each agent by setting the timer attribute
// timer minimum is 5 seconds
// timer maximum is 31 days
//
void				BBWin::LoadAgents() {
	std::pair< config_iter_t, config_iter_t > range;
	BBWinHandler 			*hand;
	
	range = m_configuration.equal_range("load");
	for ( ; range.first != range.second; ++range.first) {
		DWORD		timer = GetSeconds(m_setting[ "timer" ]);
		
		if (range.first->second["timer"].size() != 0) {
			timer = GetSeconds(range.first->second["timer"]);
			if (timer < 5 || timer > 2678400) { // invalid time
				LPCTSTR		arg[] = {range.first->second["name"].c_str(), m_setting[ "timer" ].c_str(), NULL};
				m_log->reportWarnEvent(BBWIN_SERVICE, EVENT_INVALID_TIMER, 2, arg);
				timer = GetNbr(m_setting[ "timer" ]);
			}
		}
		bbwinhandler_data_t		data = {m_hEvents, m_hCount, range.first->second["name"], range.first->second["value"], m_bbdisplay, m_setting, timer};
		
		map< std::string, BBWinHandler * >::iterator itr;
		
		itr = m_handler.find(range.first->second["name"]);
		if (itr == m_handler.end()) {
			try {
				hand = new BBWinHandler(data);
			} catch (std::bad_alloc ex) {
				continue ;
			}
			hand->start();
			m_handler.insert(pair< std::string, BBWinHandler * >(range.first->second["name"], hand));
		} else {
			LPCTSTR		arg[] = {range.first->second["name"].c_str(), NULL};
			m_log->reportErrorEvent(BBWIN_SERVICE, EVENT_ALREADY_LOADED_AGENT, 1, arg);
		}
	}
}

//
//  FUNCTION: BBWin::UnloadAgents
//
//  PURPOSE: Unload the agents
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  just delete the BBWinHandlers objects 
//
void				BBWin::UnloadAgents() {
	map<string, BBWinHandler * >::iterator itr;

	m_log->logDebug("Listing Agents Threads before Terminating.");
	for (itr = m_handler.begin(); itr != m_handler.end(); ++itr) {
		string mes = "will stop " + (*itr).second->GetName();
		m_log->logDebug(mes);
	}
	m_log->logDebug("Agents Threads Terminating.");
	for (itr = m_handler.begin(); itr != m_handler.end(); ++itr) {
		string mes = "wait for " + (*itr).second->GetName();
		m_log->logDebug(mes);
		(*itr).second->stop();
	}
	m_log->logDebug("Agents Threads Terminated.");
	for (itr = m_handler.begin(); itr != m_handler.end(); ++itr) {
		delete (*itr).second;
	}
	m_log->logDebug("Agents Threads Stopped.");
	m_handler.clear();
	m_log->logDebug("Agents Table Clear Done.");
}

//
//  FUNCTION: BBWin::WaitFor
//
//  PURPOSE: Main loop : wait for service stopping  event
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//

void						BBWin::WaitFor() {
	DWORD					dwWait;
	
	for (;;) {
		dwWait = WaitForMultipleObjects( 1, m_hEvents, FALSE, INFINITE );
			if ( dwWait == WAIT_OBJECT_0 )    // service stop signaled
				break;           
			if (dwWait == WAIT_ABANDONED_0)
				break ;
	}
}

//
//  FUNCTION: BBWin::Start
//
//  PURPOSE: Run method 
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// public entry point to run the BBWin engine
//
void 			BBWin::Start(HANDLE h) {
	try {
		Init(h);
	} catch (BBWinException ex) {
		throw ex;
	}
	try {
		LoadConfiguration();
		CheckConfiguration();
		LoadAgents();
	} catch (BBWinConfigException _ex) {
		BBWinException 		ex(_ex.getMessage().c_str());
		throw ex;
	}
	m_log->logInfo("bbwin is started.");
	LPCTSTR		argStart[] = {SZSERVICENAME, m_setting["hostname"].c_str(), NULL};
	m_log->reportInfoEvent(BBWIN_SERVICE, EVENT_SERVICE_STARTED, 2, argStart);
	try {
		WaitFor();
	} catch (BBWinException ex) {
		throw ex;
	}

}

//
//  FUNCTION: BBWin::Stop
//
//  PURPOSE: Stop the BBWin
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
// public entry point to stop the BBWin engine
//
void 			BBWin::Stop() {
	UnloadAgents();
	m_log->logInfo("bbwin is stopped.");
	LPCTSTR		argStop[] = {SZSERVICENAME, NULL};
	m_log->reportInfoEvent(BBWIN_SERVICE, EVENT_SERVICE_STOPPED, 1, argStop);
}

// BBWinException
BBWinException::BBWinException(const char* m) {
	msg = m;
}

string BBWinException::getMessage() const {
	return msg;
}
