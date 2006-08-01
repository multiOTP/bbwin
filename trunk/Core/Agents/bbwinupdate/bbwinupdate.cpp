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

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "BBWinUpdate.h"

static const BBWinAgentInfo_t 		bbwinupdateAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	"bbwinupdate",    					// agentName;
	"bbwinupdate agent : update the local configuration from central point",        // agentDescription;
	0 
};                


void 		AgentBBWinUpdate::Run() {
	HANDLE hFile; 
 
	hFile = CreateFile(TEXT("C:\\BBWinUpdate.data"),     // file to create
		GENERIC_READ | GENERIC_WRITE,          // open for writing
		FILE_SHARE_READ | FILE_SHARE_WRITE, // share
		NULL,                   // default security
		CREATE_ALWAYS,          // overwrite existing
		0,   // asynchronous I/O
		0);                  // no attr. template

	if (hFile == INVALID_HANDLE_VALUE) 
	{ 
		cout << "Could not open file " << GetLastError() << endl;
		return ;
	}

	TCHAR szName[]=TEXT("MyFileMappingObject");
	TCHAR szMsg[]=TEXT("Message from first process");
	HANDLE hMapFile;
	LPTSTR pBuf;

	hMapFile = CreateFileMapping(
                 hFile,    // use paging file
                 NULL,                    // default security 
                 PAGE_READWRITE,          // read/write access
                 0,    // max. object size 
                 FILE_MAX_CONFIG_SIZE,                // buffer size  
                 NULL);                 // name of mapping object
 
   if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) 
   { 
      cout << "Could not create file mapping object " << GetLastError() << endl;
      return;
   }
   pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,                   
                        0,                   
                        FILE_MAX_CONFIG_SIZE);           
	
	if (pBuf == NULL) 
	{ 
		  cout << "Could not map view of file " << GetLastError() << endl;
		return;
	}
	
	//cout << "BBWin Update Started" << endl;
	m_mgr.Config(m_fileName.c_str(), pBuf, FILE_MAX_CONFIG_SIZE);
	
	string test = pBuf;
	cout << test << endl;

	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(hFile);
}

AgentBBWinUpdate::AgentBBWinUpdate(IBBWinAgentManager & mgr) : 
			m_mgr(mgr),
			m_fileName("BBWin.cfg")
{
	
}

bool		AgentBBWinUpdate::Init() {
	PBBWINCONFIG	conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());

	if (conf == NULL)
		return false;
	PBBWINCONFIGRANGE range = m_mgr.GetConfigurationRange(conf, "setting");
	if (range == NULL)
		return false;
	do {
		string name, value;
		
		name = m_mgr.GetConfigurationRangeValue(range, "name");
		value = m_mgr.GetConfigurationRangeValue(range, "value");
		if (name == "filename" && value.length() > 0) {
			m_fileName = value;
		}
	}
	while (m_mgr.IterateConfigurationRange(range));
	m_mgr.FreeConfigurationRange(range);
	m_mgr.FreeConfiguration(conf);
	return true;
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentBBWinUpdate(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}

BBWIN_AGENTDECL const BBWinAgentInfo_t * GetBBWinAgentInfo() {
	return &bbwinupdateAgentInfo;
}


