#============================================================================#
#        M S  N M A K E     S O U R C E   S P E C I F I C A T I O N          #
#============================================================================#
#                                                                            #
# NAME                                                                       #
# Makefile   To automate bbwin compilation and bbwin.msi creation            #
#                                                                            #
# REVISION HISTORY                                                           #
#     11/26/2010    T.J. Yang tjyang2001@gmail.com  Version 0.9 Original.    #
#     MM/DD/YEAR    Your Name Here.                                          #
#                                                                            #
#                                                                            #
# NOTE: Ensure you have the WiX and HTML Help Workshop directories added to  #
# your PATH                                                                  #
#                                                                            #
# USAGE                                                                      #
#  1. nmake clean                                                            #
#     This will delete ..\Release\*.*, bbwin.chm and temp files in Setup.    #
#  2. nmake                                                                  #
#     This will run following targets                                        #
#      vs_build sdk_build check $(PROJ).chm $(PROJ).msi                      #
#                                                                            #
# DESCRIPTION                                                                #
# This Makefile is for MS make(nmake) to automate the bbwin src compiliation #
# and BBWin.msi generation process.                                          #
#                                                                            #
# TODOs                                                                      #
#                                                                            #
# LICENSE                                                                    #
# GNU                                                                        #
# This program is released under the GNU General Public License (GPL),       #
# version 2. See the file "COPYING" for details.                             #
#                                                                            #
# WARRANTY                                                                   #
# This program is distributed in the hope that it will be useful,            #
# but without any warranty; without even the implied warranty of             #
# merchantability or fitness for a particular purpose.                       #
#                                                                            #
# COPYRIGHT                                                                  #
#  Copyright (C)2006 Etienne GRIGNON  ( etienne.grignon@gmail.com )          #
#                                                                            #
# REFERENCES                                                                 # 
#  1. http://www.tramontana.co.hu/wix/, where I learn WiX from.              #
#                                                                            #
#                                                                            #
#============================================================================#

!include <Win32.Mak>



PROJ = BBWin 
all: vs_build check $(PROJ).chm $(PROJ).msi


# Define project specific macros
PROJ_OBJS  = UpgradeConfiguration.obj
BASE_OBJS  =
EXTRA_LIBS = msi.lib
GLOBAL_DEP =
RC_DEP     =
CL_PRE 	   =

.SUFFIXES: .wxs .wixobj

WIXCC = candle
WIXLINKER = light
#WCFLAG = /nologo -ext WixUtilExtension 
#WLFLAG = /nologo  -ext WixUIExtension 
WCFLAG = -ext WixUtilExtension 
WLFLAG = -ext WixUIExtension  -ext WixUtilExtension 
HHC = hhc
HHCFLAG = 
LANG = en

###########################
# makefile inference rules 
###########################

# WIXCC compiler generate wixobj from wxs 
.wxs.wixobj:
	$(WIXCC) $(WCFLAG) $(PROJ).wxs
 
# WIXCC linker generate msi from wixobj
.wixobj.msi:
	$(WIXLINKER) $(WLFLAG) $(PROJ).wixobj


# Inference rule for updating the object files
.cpp.obj:
    $(cc) $(CL_PRE) $(cdebug) $(cflags) $(cvarsdll) $**

# Build rule for resource file
$(PROJ).res: $(PROJ).rc $(RC_DEP)
   $(rc) $(rcflags) $(rcvars) /fo $(PROJ).res $(PROJ).rc

# Build rule for DLL
$(PROJ).dll: $(BASE_OBJS) $(PROJ_OBJS) $(PROJ).res
    $(link) $(ldebug) $(dlllflags) $(guilibsdll) \
    $(BASE_OBJS) $(PROJ_OBJS) $(EXTRA_LIBS) $(PROJ).res \
    -out:$(PROJ).dll $(MAPFILE)

# msi to depend on wixobj to trigger the compile/build.
$(PROJ).msi: $(PROJ).wixobj


# This BBWin.chm need to be made from hhc.exe for BBWin.wxs
# BBWin.chm generated from bbwin.hhp in ..\docs\en
# Use '-' to ignore return code from hhc.exe as it always returns 1.
$(PROJ).chm:
	-$(HHC) ..\doc\$(LANG)\$(PROJ).hhp

# Rules for cleaning out those old files
clean:
	erase /q /s $(PROJ).dll $(PROJ).exp \
        ..\Release\*.* \
        $(PROJ_OBJS) $(PROJ).res $(PROJ).wixobj *~ *.bak *.wixpdb  $(PROJ).msi \
         ..\doc\en\$(PROJ).chm

# use vcexpress to build bbwin.sln from command line.
vs_build:
	vcexpress ../bbwin.sln /build Release
install:
	msiexec /i $(PROJ).msi /qb ADDDEFAULT=ALL
uninstall:
	msiexec /x $(PROJ).msi /qb

check:
	wixcop $(PROJ).wxs
start:	
	net start bbwin

redo: uninstall all install start