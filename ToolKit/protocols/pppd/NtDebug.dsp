# Microsoft Developer Studio Project File - Name="NtDebug" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=NtDebug - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NtDebug.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NtDebug.mak" CFG="NtDebug - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NtDebug - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "NtDebug - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
# PROP WCE_FormatVersion ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NtDebug - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NTDEBUG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NTDEBUG_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "NtDebug - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../build/nt32/bin"
# PROP Intermediate_Dir "../../build/nt32/bin"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NTDEBUG_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /ML /W3 /Gm /GX /Zi /Od /Gf /Gy /X /I "../../include/efi" /I "../../include/efi/ia32" /I "../../include/bsd" /D "LIBMD_SUPPORT" /D "__STDC__" /D "__FreeBSD__" /D "NO_DRAND48" /D "OLD_OPTIONS" /D "EFI32" /D "EFI_NT_EMULATOR" /D "UNICODE" /D EFI_FIRMWARE=0x000A0006 /FR /Fp"../../build/nt32/bin/Ntpppd.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../../build/nt32/bin/ntpppd.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../../build/nt32/output/lib/libc/libc.lib ../../build/nt32/output/lib/libsocket/libsocket.lib ../../build/nt32/output/lib/libtty/libtty.lib ../../build/nt32/output/lib/libmd/libmd.lib ../../build/nt32/output/lib/libcrypt/libcrypt.lib /nologo /entry:"InitializeDriver" /dll /debug /machine:I386 /nodefaultlib /out:"../../build/nt32/bin/ntpppd.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "NtDebug - Win32 Release"
# Name "NtDebug - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\auth.c
# End Source File
# Begin Source File

SOURCE=.\cbcp.c
# End Source File
# Begin Source File

SOURCE=.\ccp.c
# End Source File
# Begin Source File

SOURCE=.\chap.c
# End Source File
# Begin Source File

SOURCE=.\EfiHook.c
# End Source File
# Begin Source File

SOURCE=.\fsm.c
# End Source File
# Begin Source File

SOURCE=.\ipcp.c
# End Source File
# Begin Source File

SOURCE=.\lcp.c
# End Source File
# Begin Source File

SOURCE=.\magic.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\options.c
# End Source File
# Begin Source File

SOURCE=.\stubs.c
# End Source File
# Begin Source File

SOURCE=".\sys-bsd.c"
# End Source File
# Begin Source File

SOURCE=.\upap.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
