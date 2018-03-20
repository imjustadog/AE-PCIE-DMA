# Microsoft Developer Studio Generated NMAKE File, Based on DriverMgr.dsp
!IF "$(CFG)" == ""
CFG=DriverMgr - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DriverMgr - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DriverMgr - Win32 Debug" && "$(CFG)" != "DriverMgr - Win32 Unicode Debug" && "$(CFG)" != "DriverMgr - Win32 Release MinSize" && "$(CFG)" != "DriverMgr - Win32 Release MinDependency" && "$(CFG)" != "DriverMgr - Win32 Unicode Release MinSize" && "$(CFG)" != "DriverMgr - Win32 Unicode Release MinDependency"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DriverMgr.mak" CFG="DriverMgr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DriverMgr - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DriverMgr - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DriverMgr - Win32 Release MinSize" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DriverMgr - Win32 Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DriverMgr - Win32 Unicode Release MinSize" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DriverMgr - Win32 Unicode Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DriverMgr - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\DriverMgr.dll" ".\Debug\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\DriverMgr.obj"
	-@erase "$(INTDIR)\DriverMgr.pch"
	-@erase "$(INTDIR)\DriverMgr.res"
	-@erase "$(INTDIR)\s3_1000.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\DriverMgr.dll"
	-@erase "$(OUTDIR)\DriverMgr.exp"
	-@erase "$(OUTDIR)\DriverMgr.ilk"
	-@erase "$(OUTDIR)\DriverMgr.lib"
	-@erase "$(OUTDIR)\DriverMgr.pdb"
	-@erase ".\DriverMgr.h"
	-@erase ".\DriverMgr.tlb"
	-@erase ".\DriverMgr_i.c"
	-@erase ".\Debug\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\DriverMgr.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DriverMgr.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DriverMgr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=setupapi.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\DriverMgr.pdb" /debug /machine:I386 /def:".\DriverMgr.def" /out:"$(OUTDIR)\DriverMgr.dll" /implib:"$(OUTDIR)\DriverMgr.lib" /pdbtype:sept 
DEF_FILE= \
	".\DriverMgr.def"
LINK32_OBJS= \
	"$(INTDIR)\DriverMgr.obj" \
	"$(INTDIR)\s3_1000.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DriverMgr.res"

"$(OUTDIR)\DriverMgr.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Debug
TargetPath=.\Debug\DriverMgr.dll
InputPath=.\Debug\DriverMgr.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Unicode Debug"

OUTDIR=.\DebugU
INTDIR=.\DebugU
# Begin Custom Macros
OutDir=.\DebugU
# End Custom Macros

ALL : "$(OUTDIR)\DriverMgr.dll" ".\DriverMgr.tlb" ".\DriverMgr.h" ".\DriverMgr_i.c" ".\DebugU\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\DriverMgr.obj"
	-@erase "$(INTDIR)\DriverMgr.pch"
	-@erase "$(INTDIR)\DriverMgr.res"
	-@erase "$(INTDIR)\s3_1000.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\DriverMgr.dll"
	-@erase "$(OUTDIR)\DriverMgr.exp"
	-@erase "$(OUTDIR)\DriverMgr.ilk"
	-@erase "$(OUTDIR)\DriverMgr.lib"
	-@erase "$(OUTDIR)\DriverMgr.pdb"
	-@erase ".\DriverMgr.h"
	-@erase ".\DriverMgr.tlb"
	-@erase ".\DriverMgr_i.c"
	-@erase ".\DebugU\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /Fp"$(INTDIR)\DriverMgr.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DriverMgr.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DriverMgr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\DriverMgr.pdb" /debug /machine:I386 /def:".\DriverMgr.def" /out:"$(OUTDIR)\DriverMgr.dll" /implib:"$(OUTDIR)\DriverMgr.lib" /pdbtype:sept 
DEF_FILE= \
	".\DriverMgr.def"
LINK32_OBJS= \
	"$(INTDIR)\DriverMgr.obj" \
	"$(INTDIR)\s3_1000.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DriverMgr.res"

"$(OUTDIR)\DriverMgr.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\DebugU
TargetPath=.\DebugU\DriverMgr.dll
InputPath=.\DebugU\DriverMgr.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode DLL on Windows 95 
	:end 
<< 
	

!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Release MinSize"

OUTDIR=.\ReleaseMinSize
INTDIR=.\ReleaseMinSize
# Begin Custom Macros
OutDir=.\ReleaseMinSize
# End Custom Macros

ALL : "$(OUTDIR)\DriverMgr.dll" ".\DriverMgr.tlb" ".\DriverMgr.h" ".\DriverMgr_i.c" ".\ReleaseMinSize\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\DriverMgr.obj"
	-@erase "$(INTDIR)\DriverMgr.pch"
	-@erase "$(INTDIR)\DriverMgr.res"
	-@erase "$(INTDIR)\s3_1000.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DriverMgr.dll"
	-@erase "$(OUTDIR)\DriverMgr.exp"
	-@erase "$(OUTDIR)\DriverMgr.lib"
	-@erase ".\DriverMgr.h"
	-@erase ".\DriverMgr.tlb"
	-@erase ".\DriverMgr_i.c"
	-@erase ".\ReleaseMinSize\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_DLL" /Fp"$(INTDIR)\DriverMgr.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DriverMgr.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DriverMgr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\DriverMgr.pdb" /machine:I386 /def:".\DriverMgr.def" /out:"$(OUTDIR)\DriverMgr.dll" /implib:"$(OUTDIR)\DriverMgr.lib" 
DEF_FILE= \
	".\DriverMgr.def"
LINK32_OBJS= \
	"$(INTDIR)\DriverMgr.obj" \
	"$(INTDIR)\s3_1000.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DriverMgr.res"

"$(OUTDIR)\DriverMgr.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseMinSize
TargetPath=.\ReleaseMinSize\DriverMgr.dll
InputPath=.\ReleaseMinSize\DriverMgr.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Release MinDependency"

OUTDIR=.\ReleaseMinDependency
INTDIR=.\ReleaseMinDependency
# Begin Custom Macros
OutDir=.\ReleaseMinDependency
# End Custom Macros

ALL : "$(OUTDIR)\DriverMgr2.dll" ".\ReleaseMinDependency\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\DriverMgr.obj"
	-@erase "$(INTDIR)\DriverMgr.pch"
	-@erase "$(INTDIR)\DriverMgr.res"
	-@erase "$(INTDIR)\s3_1000.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DriverMgr2.dll"
	-@erase "$(OUTDIR)\DriverMgr2.exp"
	-@erase "$(OUTDIR)\DriverMgr2.lib"
	-@erase ".\DriverMgr.h"
	-@erase ".\DriverMgr.tlb"
	-@erase ".\DriverMgr_i.c"
	-@erase ".\ReleaseMinDependency\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /Fp"$(INTDIR)\DriverMgr.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DriverMgr.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DriverMgr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=setupapi.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\DriverMgr2.pdb" /machine:I386 /def:".\DriverMgr.def" /out:"$(OUTDIR)\DriverMgr2.dll" /implib:"$(OUTDIR)\DriverMgr2.lib" 
DEF_FILE= \
	".\DriverMgr.def"
LINK32_OBJS= \
	"$(INTDIR)\DriverMgr.obj" \
	"$(INTDIR)\s3_1000.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DriverMgr.res"

"$(OUTDIR)\DriverMgr2.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseMinDependency
TargetPath=.\ReleaseMinDependency\DriverMgr2.dll
InputPath=.\ReleaseMinDependency\DriverMgr2.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Unicode Release MinSize"

OUTDIR=.\ReleaseUMinSize
INTDIR=.\ReleaseUMinSize
# Begin Custom Macros
OutDir=.\ReleaseUMinSize
# End Custom Macros

ALL : "$(OUTDIR)\DriverMgr.dll" ".\DriverMgr.tlb" ".\DriverMgr.h" ".\DriverMgr_i.c" ".\ReleaseUMinSize\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\DriverMgr.obj"
	-@erase "$(INTDIR)\DriverMgr.pch"
	-@erase "$(INTDIR)\DriverMgr.res"
	-@erase "$(INTDIR)\s3_1000.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DriverMgr.dll"
	-@erase "$(OUTDIR)\DriverMgr.exp"
	-@erase "$(OUTDIR)\DriverMgr.lib"
	-@erase ".\DriverMgr.h"
	-@erase ".\DriverMgr.tlb"
	-@erase ".\DriverMgr_i.c"
	-@erase ".\ReleaseUMinSize\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /D "_ATL_DLL" /Fp"$(INTDIR)\DriverMgr.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DriverMgr.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DriverMgr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\DriverMgr.pdb" /machine:I386 /def:".\DriverMgr.def" /out:"$(OUTDIR)\DriverMgr.dll" /implib:"$(OUTDIR)\DriverMgr.lib" 
DEF_FILE= \
	".\DriverMgr.def"
LINK32_OBJS= \
	"$(INTDIR)\DriverMgr.obj" \
	"$(INTDIR)\s3_1000.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DriverMgr.res"

"$(OUTDIR)\DriverMgr.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseUMinSize
TargetPath=.\ReleaseUMinSize\DriverMgr.dll
InputPath=.\ReleaseUMinSize\DriverMgr.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode DLL on Windows 95 
	:end 
<< 
	

!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Unicode Release MinDependency"

OUTDIR=.\ReleaseUMinDependency
INTDIR=.\ReleaseUMinDependency
# Begin Custom Macros
OutDir=.\ReleaseUMinDependency
# End Custom Macros

ALL : "$(OUTDIR)\DriverMgr.dll" ".\DriverMgr.tlb" ".\DriverMgr.h" ".\DriverMgr_i.c" ".\ReleaseUMinDependency\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\DriverMgr.obj"
	-@erase "$(INTDIR)\DriverMgr.pch"
	-@erase "$(INTDIR)\DriverMgr.res"
	-@erase "$(INTDIR)\s3_1000.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DriverMgr.dll"
	-@erase "$(OUTDIR)\DriverMgr.exp"
	-@erase "$(OUTDIR)\DriverMgr.lib"
	-@erase ".\DriverMgr.h"
	-@erase ".\DriverMgr.tlb"
	-@erase ".\DriverMgr_i.c"
	-@erase ".\ReleaseUMinDependency\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /D "_ATL_STATIC_REGISTRY" /Fp"$(INTDIR)\DriverMgr.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DriverMgr.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DriverMgr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\DriverMgr.pdb" /machine:I386 /def:".\DriverMgr.def" /out:"$(OUTDIR)\DriverMgr.dll" /implib:"$(OUTDIR)\DriverMgr.lib" 
DEF_FILE= \
	".\DriverMgr.def"
LINK32_OBJS= \
	"$(INTDIR)\DriverMgr.obj" \
	"$(INTDIR)\s3_1000.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DriverMgr.res"

"$(OUTDIR)\DriverMgr.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseUMinDependency
TargetPath=.\ReleaseUMinDependency\DriverMgr.dll
InputPath=.\ReleaseUMinDependency\DriverMgr.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode DLL on Windows 95 
	:end 
<< 
	

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("DriverMgr.dep")
!INCLUDE "DriverMgr.dep"
!ELSE 
!MESSAGE Warning: cannot find "DriverMgr.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DriverMgr - Win32 Debug" || "$(CFG)" == "DriverMgr - Win32 Unicode Debug" || "$(CFG)" == "DriverMgr - Win32 Release MinSize" || "$(CFG)" == "DriverMgr - Win32 Release MinDependency" || "$(CFG)" == "DriverMgr - Win32 Unicode Release MinSize" || "$(CFG)" == "DriverMgr - Win32 Unicode Release MinDependency"
SOURCE=.\DriverMgr.cpp

"$(INTDIR)\DriverMgr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\DriverMgr.pch" ".\DriverMgr.h" ".\DriverMgr_i.c"


SOURCE=.\DriverMgr.idl
MTL_SWITCHES=/tlb ".\DriverMgr.tlb" /h "DriverMgr.h" /iid "DriverMgr_i.c" /Oicf 

".\DriverMgr.tlb"	".\DriverMgr.h"	".\DriverMgr_i.c" : $(SOURCE) "$(INTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


SOURCE=.\DriverMgr.rc

"$(INTDIR)\DriverMgr.res" : $(SOURCE) "$(INTDIR)" ".\DriverMgr.tlb"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\s3_1000.cpp

"$(INTDIR)\s3_1000.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\DriverMgr.pch" ".\DriverMgr.h"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "DriverMgr - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\DriverMgr.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DriverMgr.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Unicode Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /Fp"$(INTDIR)\DriverMgr.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DriverMgr.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Release MinSize"

CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_DLL" /Fp"$(INTDIR)\DriverMgr.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DriverMgr.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Release MinDependency"

CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /Fp"$(INTDIR)\DriverMgr.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DriverMgr.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Unicode Release MinSize"

CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /D "_ATL_DLL" /Fp"$(INTDIR)\DriverMgr.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DriverMgr.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "DriverMgr - Win32 Unicode Release MinDependency"

CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /D "_ATL_STATIC_REGISTRY" /Fp"$(INTDIR)\DriverMgr.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DriverMgr.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

