# Microsoft Developer Studio Generated NMAKE File, Based on btreedump.dsp
!IF "$(CFG)" == ""
CFG=btreedump - Win32 Debug
!MESSAGE No configuration specified. Defaulting to btreedump - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "btreedump - Win32 Release" && "$(CFG)" != "btreedump - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "btreedump.mak" CFG="btreedump - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "btreedump - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "btreedump - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "btreedump - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\btreedump.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\indexdump.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\btreedump.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\btreedump.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\btreedump.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\btreedump.pdb" /machine:I386 /out:"$(OUTDIR)\btreedump.exe" 
LINK32_OBJS= \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\ystdio.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\indexdump.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\alloc.obj"

"$(OUTDIR)\btreedump.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "btreedump - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\btreedump.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\indexdump.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\btreedump.exe"
	-@erase "$(OUTDIR)\btreedump.ilk"
	-@erase "$(OUTDIR)\btreedump.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\btreedump.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\btreedump.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\btreedump.pdb" /debug /machine:I386 /out:"$(OUTDIR)\btreedump.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\ystdio.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\indexdump.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\alloc.obj"

"$(OUTDIR)\btreedump.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("btreedump.dep")
!INCLUDE "btreedump.dep"
!ELSE 
!MESSAGE Warning: cannot find "btreedump.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "btreedump - Win32 Release" || "$(CFG)" == "btreedump - Win32 Debug"
SOURCE=..\..\src\alloc.cpp

"$(INTDIR)\alloc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\blockfile.cpp

"$(INTDIR)\blockfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\btree.cpp

"$(INTDIR)\btree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\indexdump.c

"$(INTDIR)\indexdump.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\list.cpp

"$(INTDIR)\list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\util.cpp

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\ystdio.cpp

"$(INTDIR)\ystdio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

