# Microsoft Developer Studio Generated NMAKE File, Based on yasequery.dsp
!IF "$(CFG)" == ""
CFG=yasequery - Win32 Debug
!MESSAGE No configuration specified. Defaulting to yasequery - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "yasequery - Win32 Release" && "$(CFG)" != "yasequery - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yasequery.mak" CFG="yasequery - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yasequery - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "yasequery - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "yasequery - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\yasequery.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\avl3a.obj"
	-@erase "$(INTDIR)\avl3b.obj"
	-@erase "$(INTDIR)\bitset.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\boolquery.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\cbitfile.obj"
	-@erase "$(INTDIR)\docdb.obj"
	-@erase "$(INTDIR)\getconfig.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\locator.obj"
	-@erase "$(INTDIR)\param.obj"
	-@erase "$(INTDIR)\postfile.obj"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\queryin.obj"
	-@erase "$(INTDIR)\queryout.obj"
	-@erase "$(INTDIR)\rankedquery.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\stem.obj"
	-@erase "$(INTDIR)\tokenizer.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\yasequery.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\yasequery.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\yasequery.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\yasequery.pdb" /machine:I386 /out:"$(OUTDIR)\yasequery.exe" 
LINK32_OBJS= \
	"$(INTDIR)\alloc.obj" \
	"$(INTDIR)\avl3a.obj" \
	"$(INTDIR)\avl3b.obj" \
	"$(INTDIR)\bitset.obj" \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\boolquery.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\cbitfile.obj" \
	"$(INTDIR)\docdb.obj" \
	"$(INTDIR)\getconfig.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\locator.obj" \
	"$(INTDIR)\param.obj" \
	"$(INTDIR)\postfile.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\queryin.obj" \
	"$(INTDIR)\queryout.obj" \
	"$(INTDIR)\rankedquery.obj" \
	"$(INTDIR)\stem.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\ystdio.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\tokenizer.obj"

"$(OUTDIR)\yasequery.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "yasequery - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\yasequery.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\avl3a.obj"
	-@erase "$(INTDIR)\avl3b.obj"
	-@erase "$(INTDIR)\bitset.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\boolquery.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\cbitfile.obj"
	-@erase "$(INTDIR)\docdb.obj"
	-@erase "$(INTDIR)\getconfig.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\locator.obj"
	-@erase "$(INTDIR)\param.obj"
	-@erase "$(INTDIR)\postfile.obj"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\queryin.obj"
	-@erase "$(INTDIR)\queryout.obj"
	-@erase "$(INTDIR)\rankedquery.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\stem.obj"
	-@erase "$(INTDIR)\tokenizer.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\yasequery.exe"
	-@erase "$(OUTDIR)\yasequery.ilk"
	-@erase "$(OUTDIR)\yasequery.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\yasequery.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\yasequery.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\yasequery.pdb" /debug /machine:I386 /out:"$(OUTDIR)\yasequery.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\alloc.obj" \
	"$(INTDIR)\avl3a.obj" \
	"$(INTDIR)\avl3b.obj" \
	"$(INTDIR)\bitset.obj" \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\boolquery.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\cbitfile.obj" \
	"$(INTDIR)\docdb.obj" \
	"$(INTDIR)\getconfig.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\locator.obj" \
	"$(INTDIR)\param.obj" \
	"$(INTDIR)\postfile.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\queryin.obj" \
	"$(INTDIR)\queryout.obj" \
	"$(INTDIR)\rankedquery.obj" \
	"$(INTDIR)\stem.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\ystdio.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\tokenizer.obj"

"$(OUTDIR)\yasequery.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("yasequery.dep")
!INCLUDE "yasequery.dep"
!ELSE 
!MESSAGE Warning: cannot find "yasequery.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "yasequery - Win32 Release" || "$(CFG)" == "yasequery - Win32 Debug"
SOURCE=..\..\src\alloc.cpp

"$(INTDIR)\alloc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\avl3a.cpp

"$(INTDIR)\avl3a.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\avl3b.cpp

"$(INTDIR)\avl3b.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\bitset.cpp

"$(INTDIR)\bitset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\blockfile.cpp

"$(INTDIR)\blockfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\boolquery.cpp

"$(INTDIR)\boolquery.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\btree.cpp

"$(INTDIR)\btree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\cbitfile.cpp

"$(INTDIR)\cbitfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\docdb.cpp

"$(INTDIR)\docdb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\getconfig.cpp

"$(INTDIR)\getconfig.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\list.cpp

"$(INTDIR)\list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\locator.cpp

"$(INTDIR)\locator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\param.cpp

"$(INTDIR)\param.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\postfile.cpp

"$(INTDIR)\postfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\query.cpp

"$(INTDIR)\query.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\queryin.cpp

"$(INTDIR)\queryin.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\queryout.cpp

"$(INTDIR)\queryout.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\rankedquery.cpp

"$(INTDIR)\rankedquery.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\search.cpp

"$(INTDIR)\search.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\stem.c

"$(INTDIR)\stem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\tokenizer.cpp

"$(INTDIR)\tokenizer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\util.cpp

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\ystdio.cpp

"$(INTDIR)\ystdio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

