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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include "BBWinNet.h"


//
//  FUNCTION: BBWinNet::Open
//
//  PURPOSE: open a connection to the hobbit server
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  main function to open the socket
//
void	 BBWinNet::Open()
{
	struct addrinfo 	hints;
	struct addrinfo 	*addrptr = NULL;
	DWORD				retval;
	
	if (m_bbDisplay.length() == 0) {
		throw BBWinNetException("No bb display specified");
	}
	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    retval = getaddrinfo(m_bbDisplay.c_str(), m_port.c_str(), &hints, &addrptr);
	if (retval != 0 || addrptr == NULL) {
       throw BBWinNetException("can't understand host");
    }
    m_conn = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol);
	if (m_conn == INVALID_SOCKET) {
		freeaddrinfo(addrptr);
		throw BBWinNetException("can't create socket");
    }
    retval = connect(m_conn, addrptr->ai_addr, (int)addrptr->ai_addrlen);
	if (retval == SOCKET_ERROR) {
        closesocket(m_conn);
        m_conn = INVALID_SOCKET;
		freeaddrinfo(addrptr);
		throw BBWinNetException("can't connect to host");
    }
	freeaddrinfo(addrptr);
    addrptr = NULL;
    if (m_conn == INVALID_SOCKET) {
		freeaddrinfo(addrptr);
		throw BBWinNetException("can't connect to host");
    }
	m_connected = true;
}

//
//  FUNCTION: BBWinNet::Send
//
//  PURPOSE: send a string message to the open socket
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void 	BBWinNet::Send(const string & message) {
	DWORD			tosend = 0;
	DWORD			sent = 0;
	
	if (m_debug) // used only in bbwincmd for debugging purpose. It will soon disappear
			cout << "\nSending to " << m_bbDisplay << " : \n" << message << "\n" << endl; 
	for ( ; sent < message.length(); ) {
		DWORD			hasSent = 0;
		
		if (BB_LEN_SEND > (message.length() - sent)) {
			tosend = message.length() - sent;
		} else {
			tosend = BB_LEN_SEND;
		}
		hasSent = send(m_conn, message.c_str() + sent, tosend, 0);
		if (hasSent == SOCKET_ERROR) {
			Close();
			throw BBWinNetException("Can't send message");
		}
		sent += hasSent;
	}
}

//
//  FUNCTION: BBWinNet::Init
//
//  PURPOSE: Init the BBWinNet class by calling WSAStartup 
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  only called by the constructor
//
void	BBWinNet::Init() {
	DWORD 	wsaret;
	
	wsaret = WSAStartup(MAKEWORD(2,2), &m_wsaData);
	if(wsaret)	{
		throw BBWinNetException("Can't WSAStartup");
	}
	m_connected = false;
	m_port = BB_TCP_PORT;
	m_debug = false;
}

//
//  FUNCTION: BBWinNet
//
//  PURPOSE: Constructor with hostname value
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
BBWinNet::BBWinNet(const string & hostName) {
	
	try {
		BBWinNet();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	SetHostName(hostName);
}

//
//  FUNCTION: BBWinNet
//
//  PURPOSE: Default Constructor 
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
BBWinNet::BBWinNet() {
	try {
		Init();
	} catch (BBWinNetException ex) {
		throw ex;
	}
}


//
//  FUNCTION: ~BBWinNet
//
//  PURPOSE: Destructor
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
BBWinNet::~BBWinNet() {
	WSACleanup();
}

//
//  FUNCTION: BBWinNet::SetHostName
//
//  PURPOSE: set the hostname used in hobbit messages
//
//  PARAMETERS:
//    hostname	
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void	BBWinNet::SetHostName(const string & hostName) {
	m_hostName = hostName;
}

//
//  FUNCTION: BBWinNet::SetDebug
//
//  PURPOSE: activate the debug flag
//
//  PARAMETERS:
//    debug		flag to enable debug
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  false by default
//
void 	BBWinNet::SetDebug(bool debug) {
	m_debug = debug;
}

//
//  FUNCTION: BBWinNet::GetHostName
//
//  PURPOSE: return the hostname setting
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    hostname
//
//  COMMENTS:
//
const string	& BBWinNet::GetHostName() {
	return m_hostName;
}

//
//  FUNCTION: BBWinNet::Close
//
//  PURPOSE: close the openned socket 
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void BBWinNet::Close() {
	if (m_connected)
	{
		closesocket(m_conn);
		m_connected = false;
	}
}

//
//  FUNCTION: BBWinNet::AddBBDisplay
//
//  PURPOSE: set  the BBDisplay (used to connect)
//
//  PARAMETERS:
//    bbDisp 		hobbit server (IP or dns name)
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void	BBWinNet::SetBBDisplay(const string & bbDisp) {
	string 	BBDisplay = bbDisp;
	size_t		sep;
	
	sep = BBDisplay.find(':');
	if (sep > 0 && sep <= BBDisplay.size()) {
		string 	portStr =  BBDisplay.substr(sep + 1, BBDisplay.size() + 1);
		BBDisplay = BBDisplay.substr(0, sep);
		m_port = portStr;
	} 
	m_bbDisplay = BBDisplay;
}

//
//  FUNCTION: BBWinNet::GetBBDisplay
//
//  PURPOSE: return the BBDisplay
//
//  PARAMETERS:
//   none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
const string	&BBWinNet::GetBBDisplay() {
	return m_bbDisplay;
}


//
//  FUNCTION: BBWinNet::SetPort
//
//  PURPOSE: set the tcp port used to connect to hobbit server
//
//  PARAMETERS:
//   port 		tcp port
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void	BBWinNet::SetPort(const string & port) {
	m_port = port;
}

//
//  FUNCTION: BBWinNet::GetPort
//
//  PURPOSE: return the tcp port used to connect to hobbit server
//
//  PARAMETERS:
//   none
//
//  RETURN VALUE:
//    port 		tcp port used
//
//  COMMENTS:
//
const string &	BBWinNet::GetPort() {
	return m_port;
}

//
//  FUNCTION: BBWinNet::Status
//
//  PURPOSE: send a status hobbit message
//
//  PARAMETERS:
//   testName 		hobbit testname ( appear as a column name in hobbit server)
//   color			color used (no check is done on the color for the moment)
//   text			text of the status
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void 				BBWinNet::Status(const string & testName, const string & color, const string & text, const string & lifeTime) {
	ostringstream 	status;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	status << "status";
	if (lifeTime.length() > 0)
		status << "+" << lifeTime;
	status << " " << m_hostName << "." << testName << " " << color << " " << text;
	string res = status.str();
		try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Pager
//
//  PURPOSE: send a pager hobbit message
//  For BigBrother back compatibility
//
//  PARAMETERS:
//   testName 		hobbit testname ( appear as a column name in hobbit server)
//   color			color used (no check is done on the color for the moment)
//   text			text of the status
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void 				BBWinNet::Pager(const string & testName, const string & color, const string & text, const string & lifeTime) {
	ostringstream 	pager;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	pager << "page";
	if (lifeTime.length() > 0)
		pager << "+" << lifeTime;
	pager << " " << m_hostName << "." << testName << " " << color << " " << text;
	string res = pager.str();
		try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Notify
//
//  PURPOSE: send a notify hobbit message
//
//  PARAMETERS:
//   testName 		hobbit testname ( appear as a column name in hobbit server)
//   text			text of the notify
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Notify(const string & testName, const string & text) {
	ostringstream 	notify;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	notify << "notify";
	notify << " " << m_hostName << "." << testName << " " << text;
	string res = notify.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Data
//
//  PURPOSE: send a data hobbit message
//
//  PARAMETERS:
//   dataName 		dataname
//   text			text of the data
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Data(const string & dataName, const string & text) {
	ostringstream 	data;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	data << "data";
	data << " " << m_hostName << "." << dataName << "\n" << text;
	string res = data.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Disable
//
//  PURPOSE: send a disable hobbit message
//
//  PARAMETERS:
//   testName 		hobbit testname ( appear as a column name in hobbit server)
//  duration			duration value as 30m, or 2d
//   text			text of the status
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void 		BBWinNet::Disable(const string & testName, const string & duration, const string & text) {
	ostringstream 	disable;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	disable << "disable";
	disable << " " << m_hostName << "." << testName << " " << duration << " " << text;
	string res = disable.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Enable
//
//  PURPOSE: send a enable hobbit message
//
//  PARAMETERS:
//   testName 		hobbit testname ( appear as a column name in hobbit server)
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Enable(const string & testName) {
	ostringstream 	enable;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	enable << "enable";
	enable << " " << m_hostName << "." << testName;
	string res = enable.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Drop
//
//  PURPOSE: send a drop hobbit message (delete the host with all tests history)
//
//  PARAMETERS:
//   none
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Drop()
{
	ostringstream 	drop;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	drop << "drop";
	drop << " " << m_hostName;
	string res = drop.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Drop
//
//  PURPOSE: send a drop hobbit message ( delete the testname from the host)
//
//  PARAMETERS:
//   testName 		hobbit testname ( appear as a column name in hobbit server)
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Drop(const string & testName) {
	ostringstream 	drop;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	drop << "drop";
	drop << " " << m_hostName << " " << testName;
	string res = drop.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Rename
//
//  PURPOSE: send a rename hobbit message ( rename the hostname)
//
//  PARAMETERS:
//   newHostName 		new  hostname
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Rename(const string & newHostName) {
	ostringstream 	rename;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	rename << "rename";
	rename << " " << m_hostName << " " << newHostName;
	string res = rename.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Rename
//
//  PURPOSE: send a rename hobbit message ( rename a testname)
//
//  PARAMETERS:
//   oldTestName 	
//  newTestName
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Rename(const string & oldTestName, const string & newTestName) {
	ostringstream 	rename;
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	rename << "rename";
	rename << " " << m_hostName << " " << oldTestName << " " << newTestName;
	string res = rename.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Test
//
//  PURPOSE: test a connection to the hobbit server 
//
//  PARAMETERS:
//  none
//
//  RETURN VALUE:
//   none 
//
//  COMMENTS:
//
void		BBWinNet::Test() {
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Message
//
//  PURPOSE: send a message
//
//  PARAMETERS:
//  message 		hobbit message manually formed
//
//  RETURN VALUE:
//   string			return the string returned by the server if there is one 
//
//  COMMENTS:
//
void				BBWinNet::Message(const string & message, string & dest) {
	int				rBuf;
	char			buf[BB_LEN_RECV + 1];
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	try {
		Send(message);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	if (shutdown(m_conn, 1) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	while((rBuf = recv(m_conn, buf, BB_LEN_RECV,0)) && rBuf != SOCKET_ERROR)
	{
		buf[rBuf] = '\0';
		dest += buf;
	}
	if (rBuf == SOCKET_ERROR) {
		throw BBWinNetException("Can't receive query answer");
	}
	if (shutdown(m_conn, 0) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	if (shutdown(m_conn, 2) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Query
//
//  PURPOSE: send a query message
//
//  PARAMETERS:
//  testName 		testName queried
//
//  RETURN VALUE:
//   string			return the string returned by the server if there is one 
//
//  COMMENTS:
//
void				BBWinNet::Query(const string & testName, string & dest) {
	ostringstream 	query;
	int				rBuf;
	char			buf[BB_LEN_RECV + 1];
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	query << "query";
	query << " " << m_hostName << "." << testName;
	string res = query.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	if (shutdown(m_conn, 1) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	while((rBuf = recv(m_conn, buf, BB_LEN_RECV,0)) && rBuf != SOCKET_ERROR)
	{
		buf[rBuf] = '\0';
		dest += buf;
	}
	if (rBuf == SOCKET_ERROR) {
		throw BBWinNetException("Can't receive query answer");
	}
	if (shutdown(m_conn, 0) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	if (shutdown(m_conn, 2) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	Close();
}

//
//  FUNCTION: BBWinNet::Config
//
//  PURPOSE: send a config message
//
//  PARAMETERS:
//  filename 		filename wanted 
//
//  RETURN VALUE:
//   string			return the string returned by the server if there is one 
//
//  COMMENTS:
//
void				BBWinNet::Config(const string & fileName, string & dest) {
	ostringstream 	query;
	int				rBuf;
	char			buf[BB_LEN_RECV + 1];
	
	try {
		Open();
	} catch (BBWinNetException ex) {
		throw ex;
	}
	query << "config";
	query << " " << fileName;
	string res = query.str();
	try {
		Send(res);
	} catch (BBWinNetException ex) {
		throw ex;
	}
	if (shutdown(m_conn, 1) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	while((rBuf = recv(m_conn, buf, BB_LEN_RECV,0)) && rBuf != SOCKET_ERROR)
	{
		buf[rBuf] = '\0';
		dest += buf;
	}
	if (rBuf == SOCKET_ERROR) {
		throw BBWinNetException("Can't receive query answer");
	}
	if (shutdown(m_conn, 0) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	if (shutdown(m_conn, 2) != 0) {
		throw BBWinNetException("Can't shutdown socket");
	}
	Close();
}

// BBwinNetException
BBWinNetException::BBWinNetException(const char* m) {
	msg = m;
}

string BBWinNetException::getMessage() const {
	return msg;
}
