# Microsoft Developer Studio Generated NMAKE File, Based on btreetest.dsp
!IF "$(CFG)" == ""
CFG=btreetest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to btreetest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "btreetest - Win32 Release" && "$(CFG)" != "btreetest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "btreetest.mak" CFG="btreetest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "btreetest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "btreetest - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "btreetest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\btreetest.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\btreetest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\btreetest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\btreetest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\btreetest.pdb" /machine:I386 /out:"$(OUTDIR)\btreetest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\alloc.obj" \
	"$(INTDIR)\ystdio.obj"

"$(OUTDIR)\btreetest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "btreetest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\btreetest.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\btreetest.exe"
	-@erase "$(OUTDIR)\btreetest.ilk"
	-@erase "$(OUTDIR)\btreetest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "TEST_BTREE" /Fp"$(INTDIR)\btreetest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\btreetest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\btreetest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\btreetest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\alloc.obj" \
	"$(INTDIR)\ystdio.obj"

"$(OUTDIR)\btreetest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("btreetest.dep")
!INCLUDE "btreetest.dep"
!ELSE 
!MESSAGE Warning: cannot find "btreetest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "btreetest - Win32 Release" || "$(CFG)" == "btreetest - Win32 Debug"
SOURCE=..\..\src\alloc.cpp

"$(INTDIR)\alloc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\blockfile.cpp

"$(INTDIR)\blockfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\btree.cpp

"$(INTDIR)\btree.obj" : $(SOURCE) "$(INTDIR)"
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

