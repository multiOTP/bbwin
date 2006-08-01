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

#include "HobbitClient.h"

static const BBWinAgentInfo_t 		sampleAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	"hobbitclient",    					// agentName;
	"this is the core part for the centralized mode of BBWin",        // agentDescription;
	0									// flags
};                

void 		HobbitClient::Run() {

}

void		ExecuteExternalFile(string & sExeName, string & sArguments) {
  string	sExecute;
  
  sExecute = sExeName + " " + sArguments;
  SECURITY_ATTRIBUTES secattr; 
  ZeroMemory(&secattr,sizeof(secattr));
  secattr.nLength = sizeof(secattr);
  secattr.bInheritHandle = TRUE;
  HANDLE rPipe, wPipe;

  //Create pipes to write and read data
  CreatePipe(&rPipe,&wPipe,&secattr,0);
  //
  STARTUPINFO sInfo; 
  ZeroMemory(&sInfo,sizeof(sInfo));
  PROCESS_INFORMATION pInfo; 
  ZeroMemory(&pInfo,sizeof(pInfo));
  sInfo.cb=sizeof(sInfo);
  sInfo.dwFlags=STARTF_USESTDHANDLES;
  sInfo.hStdInput=NULL; 
  sInfo.hStdOutput=wPipe; 
  sInfo.hStdError=wPipe;
  char command[1024 + 1]; 
  SecureZeroMemory(command, sizeof(command));
  strncpy(command, sExecute.c_str(), 1024);

  //Create the process here.
  CreateProcess(0, command,0,0,TRUE,
          NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,0,0,&sInfo,&pInfo);
  CloseHandle(wPipe);

  //now read the output pipe here.
  char buf[100];
  DWORD reDword; 
  string  m_csOutput,csTemp;
  BOOL res;
  do
  {
                  res=::ReadFile(rPipe,buf,100,&reDword,0);
                  csTemp=buf;
                  m_csOutput += csTemp.substr(0, reDword);
  } while(res);
}

HobbitClient::HobbitClient(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	
}

bool			HobbitClient::Init() {
	/*PBBWINCONFIG		conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());

	if (conf == NULL)
		return false;
	PBBWINCONFIGRANGE range = m_mgr.GetConfigurationRange(conf, "setting");
	if (range == NULL)
		return false;
	for ( ; m_mgr.AtEndConfigurationRange(range); m_mgr.IterateConfigurationRange(range)) {
		string name, value;
		
		name = m_mgr.GetConfigurationRangeValue(range, "name");
		value = m_mgr.GetConfigurationRangeValue(range, "value");
		if (name == "testname") {
			m_testName = value;
		}
		if (name == "message") {
			m_message = value;
		}
	}	
	m_mgr.FreeConfigurationRange(range);
	m_mgr.FreeConfiguration(conf);*/
	return true;
}
//
//BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
//{
//	return new HobbitClient(mgr);
//}
//
//BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
//{
//	delete agent;
//}
//
//BBWIN_AGENTDECL const BBWinAgentInfo_t * GetBBWinAgentInfo() {
//	return &sampleAgentInfo;
//}
