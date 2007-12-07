//this file is part of BBWin
//Copyright (C)2007 Etienne GRIGNON  ( etienne.grignon@gmail.com )
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

#include <tchar.h>
#include <stdio.h>
#include <assert.h>
#include <lm.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "who.h"

#define MAX_NAME_STRING   1024

static const BBWinAgentInfo_t 		whoAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	"who",    					// agentName;
	"agent used to list current connected users",        // agentDescription;
	BBWIN_AGENT_CENTRALIZED_COMPATIBLE			// flags
};                

void AgentWho::PrintWin32Error(LPSTR ErrorMessage, DWORD ErrorCode) {
	LPVOID		lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, ErrorCode, 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPSTR) &lpMsgBuf, 0, NULL );
	printf("%s: %s\n", ErrorMessage, lpMsgBuf );
	LocalFree( lpMsgBuf );
}

void DisplayLogonTime(PSYSTEMTIME LogonTime) {
	TCHAR	logonDateString[MAX_PATH];
	TCHAR	logonTimeString[MAX_PATH];

	GetDateFormat( LOCALE_USER_DEFAULT, 0,
				   LogonTime, NULL, 
				   logonDateString, sizeof(logonDateString)/sizeof(TCHAR));
	GetTimeFormat( LOCALE_USER_DEFAULT, 0,
				   LogonTime, NULL, 
				   logonTimeString, sizeof(logonTimeString)/sizeof(TCHAR));
	printf("     %s %s    ", logonDateString, logonTimeString );
}

void GetSessionLogonTime( DWORD Seconds, PSYSTEMTIME LogonTime)
{
	ULARGE_INTEGER fileTimeInteger;
	FILETIME     fileTime;

	GetLocalTime( LogonTime );
	SystemTimeToFileTime( LogonTime, &fileTime );
	fileTimeInteger = *(PULARGE_INTEGER) &fileTime;
	fileTimeInteger.QuadPart -= Seconds * 10000000;
	fileTime = *(PFILETIME) &fileTimeInteger;
	FileTimeToSystemTime( &fileTime, LogonTime );
}


bool GetLocalLogonTime(HKEY hCurrentUsers, LPSTR UserSid, PSYSTEMTIME LogonTime) {
    HKEY	hKey;
	TCHAR	keyName[MAX_PATH];
    TCHAR	classBuf[1024];
    DWORD	classSize;
    DWORD	subKeys;
    DWORD	maxSubKeyLen;
    DWORD	maxClassLen;
    DWORD	values;
    DWORD	maxValueNameLen;
    DWORD	maxValueLen;
    DWORD	secDescLen;
    FILETIME lastWriteTime;
    
	_stprintf( keyName, _T("%s\\Volatile Environment"), UserSid );
    if( RegOpenKey( hCurrentUsers, keyName, &hKey ))
        return false;
    classSize = sizeof(classBuf);
    if( RegQueryInfoKey( hKey,
                         classBuf, &classSize, NULL,
                         &subKeys, &maxSubKeyLen,
                         &maxClassLen, 
                         &values, &maxValueNameLen, &maxValueLen,
                         &secDescLen,
                         &lastWriteTime )) {
        
		RegCloseKey( hKey );
        return false;
    }
    FileTimeToLocalFileTime( &lastWriteTime, &lastWriteTime );
    FileTimeToSystemTime( &lastWriteTime, LogonTime );
	RegCloseKey( hKey );
	return true;
}


bool DisplayLocalLogons( BOOLEAN ShowDetail, LPSTR UserName  )
{
	bool		first = true;
    TCHAR		userName[MAX_NAME_STRING], domainName[MAX_NAME_STRING];
    TCHAR		subKeyName[MAX_PATH];
    DWORD		subKeyNameSize, index;
    DWORD		userNameSize, domainNameSize;
    FILETIME	lastWriteTime;
    HKEY		usersKey;
    PSID		sid;
    SID_NAME_USE sidType;
    SID_IDENTIFIER_AUTHORITY authority;
	BYTE		subAuthorityCount;
    DWORD		authorityVal, revision;
	SYSTEMTIME	logonTime;
    DWORD		subAuthorityVal[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    
    //
    // Use RegConnectRegistry so that we work with remote computers
    //
	if( RegOpenKey( HKEY_USERS, NULL, &usersKey ) != ERROR_SUCCESS ) {
			return false;
	}
    //
    // Enumerate keys under HKEY_USERS
    //
    index = 0;
    subKeyNameSize = sizeof( subKeyName );
    while( RegEnumKeyEx( usersKey, index, subKeyName, &subKeyNameSize,
                         NULL, NULL, NULL, &lastWriteTime ) == ERROR_SUCCESS ) {

        //
        // Ignore the default subkey and win2K user class subkeys
        //
        if( _stricmp( subKeyName, ".default" ) &&
			!strstr( subKeyName, "Classes")) {

			//
			// Convert the textual SID into a binary SID
			//
            subAuthorityCount= sscanf( subKeyName, "S-%d-%x-%lu-%lu-%lu-%lu-%lu-%lu-%lu-%lu",
                                        &revision, &authorityVal,
                                        &subAuthorityVal[0],
                                        &subAuthorityVal[1],
                                        &subAuthorityVal[2],
                                        &subAuthorityVal[3],
                                        &subAuthorityVal[4],
                                        &subAuthorityVal[5],
                                        &subAuthorityVal[6],
                                        &subAuthorityVal[7] );

            if( subAuthorityCount >= 3 ) {

                subAuthorityCount -= 2;
                
                //
                // Note: we can only deal with authority values
                // of 4 bytes in length
                //
                authority.Value[5] = *(PBYTE) &authorityVal;
                authority.Value[4] = *((PBYTE) &authorityVal+1);
                authority.Value[3] = *((PBYTE) &authorityVal+2);
                authority.Value[2] = *((PBYTE) &authorityVal+3);
                authority.Value[1] = 0;
                authority.Value[0] = 0;

				//
                // Initialize variables for subsequent operations
                //
                sid = NULL;
                userNameSize   = MAX_NAME_STRING;
                domainNameSize = MAX_NAME_STRING;

                if( AllocateAndInitializeSid( &authority,
                                               subAuthorityCount,
                                               subAuthorityVal[0],
                                               subAuthorityVal[1],
                                               subAuthorityVal[2],
                                               subAuthorityVal[3],
                                               subAuthorityVal[4],
                                               subAuthorityVal[5],
                                               subAuthorityVal[6],
                                               subAuthorityVal[7],
                                               &sid )) {

					//
					// We can finally lookup the account name
					//
					if( LookupAccountSid( NULL,
										  sid, 
										  userName,
										  &userNameSize,
										  domainName,
										  &domainNameSize,
										  &sidType )) {

						//
						// We've successfully looked up the user name
						//
					   if( first && !UserName ) {
						   
							printf("Users logged on locally:\n");
							first = false;
					   }
					   if( !UserName || !_stricmp( UserName, userName )) {
						
						   first = false;
						   if( ShowDetail ) {

							   if( GetLocalLogonTime( usersKey, subKeyName, &logonTime )) {

									DisplayLogonTime( &logonTime );

							   } 
						   }
						   if( UserName ) 
							   printf("%s\\%s logged onto locally.\n", domainName, UserName);
						   else			  
							   printf("%s\\%s\n    ", domainName, userName );
					   } 						
					}
                }               
                if( sid ) FreeSid( sid );
            }
        }
        subKeyNameSize = sizeof( subKeyName );
        index++;
    }
	RegCloseKey( usersKey );

	if( first && !UserName ) 
		printf("No one is logged on locally.\n");
	return !first;
}

bool DisplaySessionLogons( bool ShowDetail, LPSTR UserName )
{
   LPSESSION_INFO_10 pBuf = NULL;
   LPSESSION_INFO_10 pTmpBuf;
   DWORD		dwLevel = 10;
   DWORD		dwPrefMaxLen = 0xFFFFFFFF;
   DWORD		dwEntriesRead = 0;
   DWORD		dwTotalEntries = 0;
   DWORD		dwResumeHandle = 0;
   DWORD		i;
   DWORD		dwTotalCount = 0;
   LPSTR		pszClientName = NULL;
   LPSTR		pszUserName = NULL;
   NET_API_STATUS nStatus;
   PSID			sid;
   DWORD		sidSize, domainNameSize;
   BYTE			sidBuffer[MAX_SID_SIZE];
   TCHAR		domainName[MAX_NAME_STRING];   
   SID_NAME_USE sidType;
   bool		first = true;
   SYSTEMTIME	logonTime;

   //
   // Now display session logons
   // 
   do {

      nStatus = NetSessionEnum( NULL,
                               pszClientName,
                               pszUserName,
                               dwLevel,
                               (LPBYTE*)&pBuf,
                               dwPrefMaxLen,
                               &dwEntriesRead,
                               &dwTotalEntries,
                               &dwResumeHandle);

      if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {

         if ((pTmpBuf = pBuf) != NULL) {

            for (i = 0; (i < dwEntriesRead); i++) {

               assert(pTmpBuf != NULL);

               if (pTmpBuf == NULL) {

                  fprintf(stderr, "An access violation has occurred\n");
                  break;
               }

			   //
			   // Take the name and look up a SID so that we can get full domain/user
			   // information
			   //
			   sid = (PSID) sidBuffer;
			   sidSize = sizeof( sidBuffer );
			   domainNameSize = sizeof( domainName );

			   if( pTmpBuf->sesi10_username[0] ) {

				   if( first && !UserName ) {
					   
						printf("\nUsers logged on via resource shares:\n");
						first = false;
				   }
				   if( LookupAccountName( pTmpBuf->sesi10_cname ,
										pTmpBuf->sesi10_username,
										sid,
										&sidSize,
										domainName,
										&domainNameSize,
										&sidType )) {

					   if( !UserName || !_stricmp( UserName, pTmpBuf->sesi10_username )) {

							first = false;
							if( ShowDetail ) {

								GetSessionLogonTime( pTmpBuf->sesi10_time, &logonTime );
								DisplayLogonTime( &logonTime );
							}
							if( UserName ) 
								printf("%s\\%s logged onto remotely.\n", 
												domainName, UserName);
							else   		  
								printf("%s%s\\%s\n", ShowDetail ? L"" : L"     ", 
												domainName, pTmpBuf->sesi10_username );
					   }

				   } else {
						
					   if( !UserName || !_stricmp( UserName, pTmpBuf->sesi10_username )) {

							first = false;
							if( ShowDetail ) {

								GetSessionLogonTime( pTmpBuf->sesi10_time, &logonTime );
								DisplayLogonTime( &logonTime );
							}
							if( UserName ) printf("\r\\%s logged onto remotely.\n", 
													UserName );
							else		  printf("%s\\%s\n", ShowDetail ? L"" : L"      ", 
													pTmpBuf->sesi10_username );
					   }
				   }
			   }

               pTmpBuf++;
               dwTotalCount++;
            }
         }
      } else {

		  printf("Unable to query resource logons\n");
		  first = false;
	  }

      if (pBuf != NULL) {

         NetApiBufferFree(pBuf);
         pBuf = NULL;
      }
   } while (nStatus == ERROR_MORE_DATA);

   if (pBuf != NULL)
      NetApiBufferFree(pBuf);
	if( first && !UserName ) printf("\nNo one is logged on via resource shares.\n");
	return !first;
}


void 		AgentWho::Run() {
	stringstream 	reportData;	
        
	reportData << "who monitoring agent\n" << endl;
	DisplayLocalLogons(1, NULL);
	DisplaySessionLogons(1, NULL);

	//if (m_mgr.IsCentralModeEnabled())
	//	m_mgr.ClientData(m_testName.c_str(), reportData.str().c_str());
	//else
	//	m_mgr.Status(m_testName.c_str(), "green", reportData.str().c_str());
}

AgentWho::AgentWho(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	m_testName = "who";
}

bool		AgentWho::Init() {
	PBBWINCONFIG		conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());

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
	}	
	m_mgr.FreeConfigurationRange(range);
	m_mgr.FreeConfiguration(conf);
	return true;
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentWho(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}

BBWIN_AGENTDECL const BBWinAgentInfo_t * GetBBWinAgentInfo() {
	return &whoAgentInfo;
}
