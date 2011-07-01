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

all: sample_local.dll sample_central.dll

cflags = $(cflags) /EHsc /I ../..

.cpp.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $**		
## sample dependencies

sample_local_objs = 	sample_local.obj

sample_local.res: sample_local.rc
	$(rc) $(rcflags) $(rcvars) /fo sample_local.res sample_local.rc

sample_local.dll: $(sample_local_objs) sample_local.res
    $(link) /DLL $(dllldebug) $(sample_local_objs) sample_local.res -out:sample_local.dll
	if not exist Release mkdir Release
	copy /Y sample_local.dll Release
	copy /Y sample_local.dll ..\bin

sample_central_objs = 	sample_central.obj

sample_central.res: sample_central.rc
	$(rc) $(rcflags) $(rcvars) /fo sample_central.res sample_central.rc

sample_central.dll: $(sample_central_objs) sample_central.res
    $(link) /DLL $(dllldebug) $(sample_central_objs) sample_central.res -out:sample_central.dll
	if not exist Release mkdir Release
	copy /Y sample_central.dll Release
	copy /Y sample_central.dll ..\bin

sample_objs = $(sample_central_objs) $(sample_local_objs)

clean:
	erase /Q /S $(sample_objs) *.dll *~ *.lib *.exp *.pdb *.res


