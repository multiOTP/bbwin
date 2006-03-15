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

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

#include "Utils.h"

namespace	utils {


typedef struct 	duration_s {
	LPCTSTR		letter;
	DWORD		duration;
}				duration_t;

// The LIFETIME is in minutes, unless you add an "h" (hours), "d" (days) or "w" (weeks) immediately after the number

static 	const duration_t		duration_table[] = 
{
	{"s", 1}, 			// s for seconds
	{"m", 60}, 			// m for minutes
	{"h", 3600},		// h for hours
	{"d", 86400},		// d for days
	{"w", 604800},		// w for weeks
	{NULL, 0}
};

//
//  FUNCTION: getSeconds
//
//  PURPOSE: return seconds from duration string
//
//  PARAMETERS:
//    str 		string contening the seconds number as "65", "100m", "6d" for 6 days
//
//  RETURN VALUE:
//    DWORD seconds number
//
//  COMMENTS:
// 
//
DWORD			GetSeconds(const string & str ) {
	DWORD		sec;
	size_t		let;
	string		dup = str;
	bool		noTimeIndic = true;
	
	sec = 0;
	for (int i = 0; duration_table[i].letter != NULL; i++) {
		let = str.find(duration_table[i].letter);
		if (let > 0 && let < str.size()) {
			istringstream iss(str.substr(0, let));
			iss >> sec;
			sec *= duration_table[i].duration;
			noTimeIndic = false;
			break ;
		} 
	}
	if (noTimeIndic == true) {
		istringstream iss(str.substr(0, let));
		iss >> sec;
	}
	return sec;
}

//
//  FUNCTION: GetLastErrorText
//
//  PURPOSE: copies error message text to char *
//
//  PARAMETERS:
//    lpszBuf - destination buffer
//    dwSize - size of buffer
//
//  RETURN VALUE:
//    destination buffer
//
//  COMMENTS:
//
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize )
{
   DWORD dwRet;
   LPTSTR lpszTemp = NULL;

   dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          NULL,
                          GetLastError(),
                          LANG_NEUTRAL,
                          (LPTSTR)&lpszTemp,
                          0,
                          NULL );
   // supplied buffer is not long enough
	if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) ) {
		lpszBuf[0] = TEXT('\0');
	} else {
		lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
		strcpy(lpszBuf, lpszTemp);
	}
	if ( lpszTemp )
		LocalFree((HLOCAL) lpszTemp );
	return lpszBuf;
}


//
//  FUNCTION: GetLastErrorString
//
//  PURPOSE: copies error message text to string
//
//  PARAMETERS:
//    str - string destination
//
//  RETURN VALUE:
//   none
//
//  COMMENTS:
// 
//  wrapper method to GetLastErrorText
// 
#define		MAX_OUTPUT_ERROR			1024

void		GetLastErrorString(string & str) {
	char	buf[MAX_OUTPUT_ERROR + 1];
	
	GetLastErrorText(buf, MAX_OUTPUT_ERROR);
	str = buf;
}


//
//  FUNCTION: getNumber
//
//  PURPOSE: return number from a string
//
//  PARAMETERS:
//    str 		string contening the  number
//
//  RETURN VALUE:
//    DWORD 	number
//
//  COMMENTS:
// 
//
DWORD			GetNbr(const string & str ) {
	DWORD		nbr;

	istringstream iss(str);
	iss >> nbr;
	return nbr;
}

//
//  FUNCTION: getEnvironmentVariable
//
//  PURPOSE: return number from a string
//
//  PARAMETERS:
//    str 		string contening the  number
//
//  RETURN VALUE:
//    DWORD 	number
//
//  COMMENTS:
// 
//
void			GetEnvironmentVariable(const string & varname, string & dest) {
	DWORD 		dwRet;
	char		buf[MAX_PATH + 1];
 
	dwRet = ::GetEnvironmentVariable(varname.c_str(), buf, MAX_PATH);
	if (dwRet != 0) {
		dest = buf;
	}
}


} // namespace utils



