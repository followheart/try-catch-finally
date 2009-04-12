# Microsoft Developer Studio Generated NMAKE File, Based on yasehtmcnv.dsp
!IF "$(CFG)" == ""
CFG=yasehtmcnv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to yasehtmcnv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "yasehtmcnv - Win32 Release" && "$(CFG)" != "yasehtmcnv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yasehtmcnv.mak" CFG="yasehtmcnv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yasehtmcnv - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "yasehtmcnv - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "yasehtmcnv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\yasehtmcnv.exe"


CLEAN :
	-@erase "$(INTDIR)\htmconvert.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\yasehtmcnv.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../libxml2-2.4.15/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\yasehtmcnv.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\yasehtmcnv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib libxml2_a.lib wsock32.lib user32.lib advapi32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\yasehtmcnv.pdb" /machine:I386 /out:"$(OUTDIR)\yasehtmcnv.exe" /libpath:"../../libxml2-2.4.15/win32/dsp/libxml2" 
LINK32_OBJS= \
	"$(INTDIR)\htmconvert.obj"

"$(OUTDIR)\yasehtmcnv.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "yasehtmcnv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\yasehtmcnv.exe"


CLEAN :
	-@erase "$(INTDIR)\htmconvert.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\yasehtmcnv.exe"
	-@erase "$(OUTDIR)\yasehtmcnv.ilk"
	-@erase "$(OUTDIR)\yasehtmcnv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gm /GX /ZI /Od /I "../../libxml2-2.4.15/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\yasehtmcnv.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\yasehtmcnv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libxml2_a.lib wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\yasehtmcnv.pdb" /debug /machine:I386 /out:"$(OUTDIR)\yasehtmcnv.exe" /pdbtype:sept /libpath:"../../libxml2-2.4.15/win32/dsp/libxml2" 
LINK32_OBJS= \
	"$(INTDIR)\htmconvert.obj"

"$(OUTDIR)\yasehtmcnv.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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
!IF EXISTS("yasehtmcnv.dep")
!INCLUDE "yasehtmcnv.dep"
!ELSE 
!MESSAGE Warning: cannot find "yasehtmcnv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "yasehtmcnv - Win32 Release" || "$(CFG)" == "yasehtmcnv - Win32 Debug"
SOURCE=..\..\src\htmconvert.c

"$(INTDIR)\htmconvert.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

