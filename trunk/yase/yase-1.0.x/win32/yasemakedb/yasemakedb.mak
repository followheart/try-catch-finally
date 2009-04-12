# Microsoft Developer Studio Generated NMAKE File, Based on yasemakedb.dsp
!IF "$(CFG)" == ""
CFG=yasemakedb - Win32 Debug
!MESSAGE No configuration specified. Defaulting to yasemakedb - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "yasemakedb - Win32 Release" && "$(CFG)" != "yasemakedb - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yasemakedb.mak" CFG="yasemakedb - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yasemakedb - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "yasemakedb - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "yasemakedb - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\yasemakedb.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\avl3a.obj"
	-@erase "$(INTDIR)\avl3b.obj"
	-@erase "$(INTDIR)\bitfile.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\compress.obj"
	-@erase "$(INTDIR)\dlmalloc.obj"
	-@erase "$(INTDIR)\docdb.obj"
	-@erase "$(INTDIR)\getconfig.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt1.obj"
	-@erase "$(INTDIR)\getopt_init.obj"
	-@erase "$(INTDIR)\getword.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\locator.obj"
	-@erase "$(INTDIR)\makedb.obj"
	-@erase "$(INTDIR)\param.obj"
	-@erase "$(INTDIR)\stem.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xmlparser.obj"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\yasemakedb.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../libxml2-2.4.15/include" /I "../wget/src" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /D "YASEMAKEDB" /D "USE_WGET" /D "USE_LIBXML" /Fp"$(INTDIR)\yasemakedb.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\yasemakedb.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib wget.lib libxml2_a.lib wsock32.lib user32.lib advapi32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\yasemakedb.pdb" /machine:I386 /out:"$(OUTDIR)\yasemakedb.exe" /libpath:"../../libxml2-2.4.15/win32/dsp/libxml2" /libpath:"../../wget-1.5.3/windows/wget" 
LINK32_OBJS= \
	"$(INTDIR)\alloc.obj" \
	"$(INTDIR)\avl3a.obj" \
	"$(INTDIR)\avl3b.obj" \
	"$(INTDIR)\bitfile.obj" \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\compress.obj" \
	"$(INTDIR)\docdb.obj" \
	"$(INTDIR)\getconfig.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\getopt1.obj" \
	"$(INTDIR)\getopt_init.obj" \
	"$(INTDIR)\getword.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\locator.obj" \
	"$(INTDIR)\makedb.obj" \
	"$(INTDIR)\param.obj" \
	"$(INTDIR)\stem.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\xmlparser.obj" \
	"$(INTDIR)\ystdio.obj" \
	"$(INTDIR)\dlmalloc.obj"

"$(OUTDIR)\yasemakedb.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "yasemakedb - Win32 Debug"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\yasemakedb.exe"


CLEAN :
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\avl3a.obj"
	-@erase "$(INTDIR)\avl3b.obj"
	-@erase "$(INTDIR)\bitfile.obj"
	-@erase "$(INTDIR)\blockfile.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\compress.obj"
	-@erase "$(INTDIR)\dlmalloc.obj"
	-@erase "$(INTDIR)\docdb.obj"
	-@erase "$(INTDIR)\getconfig.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt1.obj"
	-@erase "$(INTDIR)\getopt_init.obj"
	-@erase "$(INTDIR)\getword.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\locator.obj"
	-@erase "$(INTDIR)\makedb.obj"
	-@erase "$(INTDIR)\param.obj"
	-@erase "$(INTDIR)\stem.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xmlparser.obj"
	-@erase "$(INTDIR)\ystdio.obj"
	-@erase "$(OUTDIR)\yasemakedb.exe"
	-@erase "$(OUTDIR)\yasemakedb.ilk"
	-@erase "$(OUTDIR)\yasemakedb.pdb"

CPP_PROJ=/nologo /MD /W2 /GX /Zi /Od /Ob1 /Gf /Gy /I "../../libxml2-2.4.15/include" /I "../wget/src" /D "_DEBUG" /D "HAVE_CONFIG_H" /D "YASEMAKEDB" /D "USE_WGET" /D "USE_LIBXML" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USE_DL_PREFIX" /D "DEBUG" /Fp"$(INTDIR)\yasemakedb.pch" /YX /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\yasemakedb.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wget.lib libxml2_a.lib wsock32.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\yasemakedb.pdb" /debug /machine:I386 /out:"$(OUTDIR)\yasemakedb.exe" /pdbtype:sept /libpath:"../../libxml2-2.4.15/win32/dsp/libxml2" /libpath:"../../wget-1.5.3/windows/wget" 
LINK32_OBJS= \
	"$(INTDIR)\alloc.obj" \
	"$(INTDIR)\avl3a.obj" \
	"$(INTDIR)\avl3b.obj" \
	"$(INTDIR)\bitfile.obj" \
	"$(INTDIR)\blockfile.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\compress.obj" \
	"$(INTDIR)\docdb.obj" \
	"$(INTDIR)\getconfig.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\getopt1.obj" \
	"$(INTDIR)\getopt_init.obj" \
	"$(INTDIR)\getword.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\locator.obj" \
	"$(INTDIR)\makedb.obj" \
	"$(INTDIR)\param.obj" \
	"$(INTDIR)\stem.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\xmlparser.obj" \
	"$(INTDIR)\ystdio.obj" \
	"$(INTDIR)\dlmalloc.obj"

"$(OUTDIR)\yasemakedb.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("yasemakedb.dep")
!INCLUDE "yasemakedb.dep"
!ELSE 
!MESSAGE Warning: cannot find "yasemakedb.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "yasemakedb - Win32 Release" || "$(CFG)" == "yasemakedb - Win32 Debug"
SOURCE=..\..\src\alloc.c

"$(INTDIR)\alloc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\avl3a.c

"$(INTDIR)\avl3a.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\avl3b.c

"$(INTDIR)\avl3b.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\bitfile.c

"$(INTDIR)\bitfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\blockfile.c

"$(INTDIR)\blockfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\btree.c

"$(INTDIR)\btree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\compress.c

"$(INTDIR)\compress.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\dlmalloc.c

"$(INTDIR)\dlmalloc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\docdb.c

"$(INTDIR)\docdb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\getconfig.c

"$(INTDIR)\getconfig.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\getopt.c

"$(INTDIR)\getopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\getopt1.c

"$(INTDIR)\getopt1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\getopt_init.c

"$(INTDIR)\getopt_init.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\getword.c

"$(INTDIR)\getword.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\list.c

"$(INTDIR)\list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\locator.c

"$(INTDIR)\locator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\makedb.c

"$(INTDIR)\makedb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\param.c

"$(INTDIR)\param.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\stem.c

"$(INTDIR)\stem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\util.c

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\xmlparser.c

"$(INTDIR)\xmlparser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\ystdio.c

"$(INTDIR)\ystdio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

