#this file is part of BBWin
#Copyright (C)2006 Etienne GRIGNON  ( etienne.grignon@gmail.com )
#
#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#as published by the Free Software Foundation; either
#version 2 of the License, or (at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

!include <Win32.Mak>

all: who.dll

BOOST_INC_PATH = /I "$(BOOSTINSTALLDIR)"
BOOST_LIB_PATH = /LIBPATH:"$(BOOSTINSTALLDIR)\lib"
BOOST_LIB = $(BOOST_LIB_PATH) libboost_date_time-vc90-mt-s-1_35.lib

cflags = $(cflags) /EHsc /I ../.. $(BOOST_INC_PATH)

.cpp.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $**		

## who dependencies

who_objs = 	who.obj

who.res: who.rc
	$(rc) $(rcflags) $(rcvars) /fo who.res who.rc

who.dll: $(who_objs) who.res
    $(link) /DLL $(dllldebug) $(who_objs) who.res -out:who.dll  $(BOOST_LIB) Advapi32.lib Netapi32.lib
	if not exist ..\..\..\Release mkdir ..\..\..\Release
	copy /Y who.dll ..\..\..\Release

clean:
	erase /Q /S $(who_objs)


