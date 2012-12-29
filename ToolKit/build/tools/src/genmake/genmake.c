/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  genmake.c

Abstract:

  Utility used to generate makefiles for building EFI 1.1 components.
  
--*/  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>   // for mkdir()
#include <errno.h>
#include <stdlib.h>     // for getenv()
#include <unistd.h>     // for getcwd()

#include "syslib.h"

#include "efi.h"

#include "efilib.h"

#define MAX_FILE_NAME   500

char   BuildDir[MAX_FILE_NAME];        // e.g. ..\build\ntdbg
char*  EfiSource;                      // e.g. d:\source\efi (with or without drive letter)
char*  BuildPath;                      // from EfiSource (e.g. build\ntdbg)

typedef struct {
    LIST_ENTRY      Link;
    char*          ProcDir;
    char           Filename[1];        // without extension
} A_FILE;

typedef struct {
    LIST_ENTRY      Link;
    char           Line[1];
} A_LINE;

typedef struct {
    LIST_ENTRY      C;                  // C files
    LIST_ENTRY      S;                  // S files IPF EM code
    LIST_ENTRY      A;                  // Asm files
    LIST_ENTRY      H;                  // H files
    LIST_ENTRY      INC;                // INC files
} FILE_LIST;

typedef struct {
    FILE_LIST       Comm;               // Common files
    FILE_LIST       Ia32;               // Ia32 files
    FILE_LIST       Ipf;                // Ipf files
    FILE_LIST       Ebc;                // EBC files
    LIST_ENTRY      Includes;
    LIST_ENTRY      Libraries;
    LIST_ENTRY      NMake;
} MAKE_INFO;


//
//
//

VOID
Init (
    VOID
    );

VOID
Done (
    VOID
    );

VOID
ProcessDir (
    IN UINTN    NameIndex,
    IN UINTN    DirLevel    
    );

VOID
ProcessMakeInfo (
    IN char*    DirName,
    IN UINTN    DirLevel
    );


//
//
//


int
main (
  int     argc,
  char    *argv[]
  )
{
  Init ();
  ProcessDir (0, 1);
  Done ();
  return 0;
}

VOID
Done (
    VOID
    )
{
    FILE    *fp;
    char   FileName[MAX_FILE_NAME];

    sprintf (FileName, "%s\\output\\makedone", BuildDir);
    fp = fopen (FileName, "w+");
    if (!fp) {
        printf ("Failed to create %s\n", FileName);
        getcwd(BuildDir, sizeof(BuildDir));
        printf ("cwd %s\n", BuildDir);
        exit (1);
    }
    fclose (fp);

    sprintf (FileName, "%s\\bin", BuildDir);
    mkdir (FileName,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    printf ("Make files generated\n");
}


VOID
Init (
    VOID
    )
{
    FILE        *Fp;
    char        *pDest;
    int         result;

    // Verify we are at a build point
    Fp = fopen ("master.mak", "r");
    if (!Fp) {
        printf ("genmak not run from build point\n");
        exit (1);
    }
    fclose (Fp);

    // Get the current directory and EFI_SOURCE root
    EfiSource = getenv("EFI_SOURCE");
    if (!EfiSource) {
        printf ("EFI_SOURCE enviroment variable not set\n");
        exit (1);
    }

    // _getcwd returns with drive letter
    getcwd(BuildDir, sizeof(BuildDir));

    // if EfiSource has drive letter, then we can index into
    // builddir array with strlen of efisource
    // if EfiSource doesn't have drive letter, then strip the
    // drive letter from builddir as well, so that we can
    // index to efisource properly to get buildpath
    result = 0;
    pDest = strchr(EfiSource, ':');
    if(pDest == NULL) {
        // EfiSource does not have a drive letter
        pDest = strchr(BuildDir,':');
        result = pDest - BuildDir + 1;
    }
    
    // Advance to the buildpath directory
    BuildPath = BuildDir + strlen(EfiSource) + result;

    // Set the current dir to be the EFI_SOURCE root
    result = chdir (EfiSource);
    if (result) {
        printf ("Could not find EFI_SOURCE\n");
    }
}


VOID
ProcessDir (
  IN UINTN                NameIndex,
  IN UINTN                DirLevel
  )
{
    DIR                     *DirHandle;
    static LIN32_FIND_DATA  FileData;
    static char            NameBuffer[100];

    NameBuffer[NameIndex] = 0;

    // Don't process $(EFI_SOURCE)\build
    if (strcmp (NameBuffer, "build") == 0) {
        return ;
    }

    //
    ProcessMakeInfo(NameBuffer, DirLevel);

    //
    // Process directories
    //

    DirHandle = FindFirstFile ("*", &FileData);
    do {
        // must be a directory
        if (S_ISDIR(FileData.dwFileAttributes)) {
            if (strcmp (FileData.cFileName, ".") && strcmp (FileData.cFileName, "..")) {
                // put entry in the directory list
                if (chdir (FileData.cFileName) == 0) {
                    _strlwr (FileData.cFileName);
                    if (NameIndex) {
                        sprintf (NameBuffer+NameIndex, "\\%s", FileData.cFileName);
                    } else {
                        sprintf (NameBuffer, "%s", FileData.cFileName);
                    }

                    ProcessDir (strlen (NameBuffer), DirLevel + 1);
                    NameBuffer[NameIndex] = 0;

                    chdir ("..");
                }
            }
        }
    } while (FindNextFile (DirHandle, &FileData)) ;
    FindClose (DirHandle);
}



VOID
InitMakeInfo (
    IN OUT MAKE_INFO    *MakInfo
    )
{
    InitializeListHead (&MakInfo->Comm.C);
    InitializeListHead (&MakInfo->Comm.S);
    InitializeListHead (&MakInfo->Comm.A);
    InitializeListHead (&MakInfo->Comm.H);
    InitializeListHead (&MakInfo->Comm.INC);

    InitializeListHead (&MakInfo->Ia32.C);
    InitializeListHead (&MakInfo->Ia32.S);
    InitializeListHead (&MakInfo->Ia32.A);
    InitializeListHead (&MakInfo->Ia32.H);
    InitializeListHead (&MakInfo->Ia32.INC);

    InitializeListHead (&MakInfo->Ipf.C);
    InitializeListHead (&MakInfo->Ipf.S);
    InitializeListHead (&MakInfo->Ipf.A);
    InitializeListHead (&MakInfo->Ipf.H);
    InitializeListHead (&MakInfo->Ipf.INC);

    InitializeListHead (&MakInfo->Ebc.C);
    InitializeListHead (&MakInfo->Ebc.S);
    InitializeListHead (&MakInfo->Ebc.A);
    InitializeListHead (&MakInfo->Ebc.H);
    InitializeListHead (&MakInfo->Ebc.INC);

    InitializeListHead (&MakInfo->Includes);
    InitializeListHead (&MakInfo->Libraries);
    InitializeListHead (&MakInfo->NMake);
}

VOID
FreeMakeInfoList (
    IN LIST_ENTRY      *Head
    )
{
    LIST_ENTRY         *Link, *Last;

    Link=Head->Flink; 
    while (Link != Head->Flink) {
        Last = Link;
        Link = Link->Flink;
        RemoveEntryList(Last);
        free (Last);
    }
}


VOID
FreeMakeInfo (
    IN OUT MAKE_INFO    *MakInfo
    )
{
    FreeMakeInfoList (&MakInfo->Comm.C);
    FreeMakeInfoList (&MakInfo->Comm.S);
    FreeMakeInfoList (&MakInfo->Comm.A);
    FreeMakeInfoList (&MakInfo->Comm.H);
    FreeMakeInfoList (&MakInfo->Comm.INC);

    FreeMakeInfoList (&MakInfo->Ia32.C);
    FreeMakeInfoList (&MakInfo->Ia32.S);
    FreeMakeInfoList (&MakInfo->Ia32.A);
    FreeMakeInfoList (&MakInfo->Ia32.H);
    FreeMakeInfoList (&MakInfo->Ia32.INC);

    FreeMakeInfoList (&MakInfo->Ipf.C);
    FreeMakeInfoList (&MakInfo->Ipf.S);
    FreeMakeInfoList (&MakInfo->Ipf.A);
    FreeMakeInfoList (&MakInfo->Ipf.H);
    FreeMakeInfoList (&MakInfo->Ipf.INC);

    FreeMakeInfoList (&MakInfo->Ebc.C);
    FreeMakeInfoList (&MakInfo->Ebc.S);
    FreeMakeInfoList (&MakInfo->Ebc.A);
    FreeMakeInfoList (&MakInfo->Ebc.H);
    FreeMakeInfoList (&MakInfo->Ebc.INC);

    FreeMakeInfoList (&MakInfo->Includes);
    FreeMakeInfoList (&MakInfo->Libraries);
    FreeMakeInfoList (&MakInfo->NMake);
}

BOOLEAN
CopyFileData (
    IN FILE     *FpIn,
    IN FILE     *FpOut
    )
{
    char       s[1000];


    while (fgets (s, sizeof(s)-1, FpIn)) {
        fputs (s, FpOut);
    }

    return FALSE;
}


BOOLEAN
ParseInput (
  IN FILE             *FpIn,
  IN OUT MAKE_INFO    *MakInfo
  )
{
    char              *p, *p1, *p2;
    char              *ProcDir, *DefProcDir;
    LIST_ENTRY         *FList, *LineList;
    FILE_LIST           *DefFileList, *FileList;
    A_LINE              *LInfo;
    A_FILE              *FInfo;
    char               s[1000];

    DefFileList = NULL;
    LineList = NULL;
    while (fgets (s, sizeof(s)-1, FpIn)) {
        // strip white space
        for (p=s; *p  && *p <= ' '; p++) ;

        p2 = p;
        for (p1=p; *p1; p1++) {
            if (*p1 > ' ') {
                p2 = p1;
            }
        }
        p2[1] = 0;

        if (p[0] == 0 || p[0] == '#') {
            continue;
        }

        if (p[0] == '[') {
            
            DefFileList = NULL;
            LineList = NULL;

            DefProcDir = "";
            if (strcasecmp (p, "[sources]") == 0) {
                DefFileList = &MakInfo->Comm;

            } else if (strcasecmp (p, "[ia32sources]") == 0) {
                DefFileList = &MakInfo->Ia32;
                DefProcDir = "Ia32\\";

            } else if (strcasecmp (p, "[ipfsources]") == 0) {
                DefFileList = &MakInfo->Ipf;
                DefProcDir = "Ipf\\";

            } else if (strcasecmp (p, "[ebcsources]") == 0) {
                DefFileList = &MakInfo->Ebc;
                DefProcDir = "Ebc\\";

            } else if (strcasecmp (p, "[includes]") == 0) {
                LineList = &MakInfo->Includes;

            } else if (strcasecmp (p, "[libraries]") == 0) {
                LineList = &MakInfo->Libraries;

            } else if (strcasecmp (p, "[nmake]") == 0) {
                LineList = &MakInfo->NMake;

            } else {
                printf ("FAILED: Unknown section in make.inf '%s'\n", p);
                return TRUE;

            }
            continue;
        }   

        // save data
        if (DefFileList) {
            _strlwr (p);

            // check if filename is for a specific processor type
            // and override to default 
            
            FileList = DefFileList;
            ProcDir = DefProcDir;
            if (strstr (p, "\\ia32\\")) {
                FileList = &MakInfo->Ia32;
                ProcDir = "";
            } 
            if (strstr (p, "\\ipf\\")) {
                FileList = &MakInfo->Ipf;
                ProcDir = "";
            }
            if (strstr (p, "\\ebc\\")) {
                FileList = &MakInfo->Ebc;
                ProcDir = "";
            }

            // find extension for this file
            p2 = ".";
            for (p1=p; *p1; p1++) {
                if (*p1 == '.') {
                    p2 = p1;
                }
            }

            // bin the file based on it's extension
            if (strcasecmp (p2, ".c") == 0) {
                FList = &FileList->C;
            } else if (strcasecmp (p2, ".s") == 0) {
                FList = &FileList->S;
            } else if (strcasecmp (p2, ".h") == 0) {
                FList = &FileList->H;
            } else if (strcasecmp (p2, ".inc") == 0) {
                FList = &FileList->INC;
            } else if (strcasecmp (p2, ".asm") == 0) {
                FList = &FileList->A;
            } else {
                printf ("  FAILED: Unknown source file extension type '%s'\n", p);
                return TRUE;
            }
            
            *p2 = 0;
            FInfo = malloc (sizeof(A_FILE) + strlen (p) + 1);
            FInfo->ProcDir = ProcDir;
            strcpy (FInfo->Filename, p);
            InsertTailList (FList, &FInfo->Link);
            continue;
        }
        
        if (LineList) {
			if (LineList == &MakInfo->NMake && *p != '!')
			{
				p1 = p;
				while (*p1 && *p1 != ' ' && *p1 != '=' && *p1 != ':') p1++;
				while (*p1 && *p1 == ' ') p1++;
				if (*p1 != '=' && *p1 != ':')
					p = s;
			}
            LInfo = malloc (sizeof(A_LINE) + strlen (p) + 1);
            strcpy (LInfo->Line, p);
            InsertTailList (LineList, &LInfo->Link);
            continue;
        }

        printf ("FAILED: Unknown data '%s'\n", p);
        return TRUE;
    }

    return FALSE;
}

VOID
DumpFileInfo (
    IN FILE         *FpOut,
    IN FILE_LIST    *FileList
    )
{
    LIST_ENTRY     *Link;
    A_FILE          *FInfo;


    // Each C file depends on each OBJ file
    for (Link=FileList->C.Flink; Link != &FileList->C; Link=Link->Flink) {
        FInfo = _CR (Link, A_FILE, Link);

        fprintf (FpOut, "$(BUILD_DIR)\\%s%s.o : $(SOURCE_DIR)\\%s%s.c $(INC_DEPS)\n",
            FInfo->ProcDir, FInfo->Filename,
            FInfo->ProcDir, FInfo->Filename
            );

        fprintf (FpOut, "    $(CC) $(C_FLAGS) $(MODULE_CFLAGS) /c $(SOURCE_DIR)\\%s%s.c /Fo$@ /FR$(@R).SBR\n",
            FInfo->ProcDir, FInfo->Filename
            );

        fprintf (FpOut, "\n");
    }

    fprintf (FpOut, "\n");

    // Append to build object list
    if (!IsListEmpty (&FileList->C)) {
        fprintf (FpOut, "OBJECTS = $(OBJECTS) \\\n");
        for (Link=FileList->C.Flink; Link != &FileList->C; Link=Link->Flink) {
            FInfo = _CR (Link, A_FILE, Link);
            fprintf (FpOut, "    $(BUILD_DIR)\\%s%s.o \\\n", FInfo->ProcDir, FInfo->Filename);
        }
    }

    fprintf (FpOut, "\n\n");

    // Each ASM file depends on each OBJ file
    for (Link=FileList->A.Flink; Link != &FileList->A; Link=Link->Flink) {
        FInfo = _CR (Link, A_FILE, Link);

        fprintf (FpOut, "$(BUILD_DIR)\\%s%s.o : $(SOURCE_DIR)\\%s%s.asm $(INC_DEPS)\n",
            FInfo->ProcDir, FInfo->Filename,
            FInfo->ProcDir, FInfo->Filename
            );

        fprintf (FpOut, "    $(MASM) $(MASM_FLAGS) $(MODULE_MASMFLAGS) /Fo$@ $(SOURCE_DIR)\\%s%s.asm\n",
            FInfo->ProcDir, FInfo->Filename
            );

        fprintf (FpOut, "\n");
    }

    fprintf (FpOut, "\n");

    // Append to build object list
    if (!IsListEmpty (&FileList->A)) {
        fprintf (FpOut, "OBJECTS = $(OBJECTS) \\\n");
        for (Link=FileList->A.Flink; Link != &FileList->A; Link=Link->Flink) {
            FInfo = _CR (Link, A_FILE, Link);
            fprintf (FpOut, "    $(BUILD_DIR)\\%s%s.o \\\n",             
                FInfo->ProcDir, FInfo->Filename
                );
        }
    }

    fprintf (FpOut, "\n\n");


    // Each S file depends on each OBJ file
    for (Link=FileList->S.Flink; Link != &FileList->S; Link=Link->Flink) {
        FInfo = _CR (Link, A_FILE, Link);

        fprintf (FpOut, "$(BUILD_DIR)\\%s%s.o : $(SOURCE_DIR)\\%s%s.s $(INC_DEPS)\n",
            FInfo->ProcDir, FInfo->Filename,
            FInfo->ProcDir, FInfo->Filename
            );

        fprintf (FpOut, "    $(CC) $(CFLAGS_P) $(SOURCE_DIR)\\%s%s.s > $(BUILD_DIR)\\%s%s.pro\n",
            FInfo->ProcDir, FInfo->Filename,
            FInfo->ProcDir, FInfo->Filename
            );        

        fprintf (FpOut, "    $(ASM) $(AFLAGS) $(MODULE_AFLAGS) $(BUILD_DIR)\\%s%s.pro\n",
            FInfo->ProcDir, FInfo->Filename
            );

        fprintf (FpOut, "    del $(BUILD_DIR)\\%s%s.pro\n",
            FInfo->ProcDir, FInfo->Filename
            );

        fprintf (FpOut, "\n");
    }

    fprintf (FpOut, "\n");

    // Append to build object list
    if (!IsListEmpty (&FileList->S)) {
        fprintf (FpOut, "OBJECTS = $(OBJECTS) \\\n");
        for (Link=FileList->S.Flink; Link != &FileList->S; Link=Link->Flink) {
            FInfo = _CR (Link, A_FILE, Link);
            fprintf (FpOut, "    $(BUILD_DIR)\\%s%s.o \\\n",             
                FInfo->ProcDir, FInfo->Filename
                );
        }
    }

    fprintf (FpOut, "\n\n");
}


VOID
DumpIncludeInfo (
    IN FILE         *FpOut,
    IN char*       DirName,
    IN FILE_LIST    *FileList
    )
{
    LIST_ENTRY     *Link;
    A_FILE          *FInfo;

    fprintf (FpOut, "INC_DEPS = $(INC_DEPS) \\\n");

    // Append to INC_DEPS list
    if (!IsListEmpty (&FileList->H)) {
        for (Link=FileList->H.Flink; Link != &FileList->H; Link=Link->Flink) {
            FInfo = _CR (Link, A_FILE, Link);
            fprintf (FpOut, "    $(EFI_SOURCE)\\%s\\%s%s.h \\\n", DirName, FInfo->ProcDir, FInfo->Filename);
        }
    }

    // Append to INC_DEPS list
    if (!IsListEmpty (&FileList->INC)) {
        for (Link=FileList->INC.Flink; Link != &FileList->INC; Link=Link->Flink) {
            FInfo = _CR (Link, A_FILE, Link);
            fprintf (FpOut, "    $(EFI_SOURCE)\\%s\\%s%s.inc \\\n", DirName, FInfo->ProcDir, FInfo->Filename);
        }
    }

    fprintf (FpOut, "\n\n");
}

VOID
DumpHeader (
    IN FILE     *Fp,
    IN char*    Line
    )
{
    fprintf (Fp, "\n");
    fprintf (Fp, "#\n");
    if (Line) {
        fprintf (Fp, "# %s\n", Line);
    } else {
        fprintf (Fp, "# This is a machine generated file - DO NOT EDIT\n");
        fprintf (Fp, "#    Generated by genmake.exe from component make.inf\n\n");
        fprintf (Fp, "#    Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved\n");
        fprintf (Fp, "#    This software and associated documentation (if any) is furnished\n");
        fprintf (Fp, "#    under a license and may only be used or copied in accordance\n");
        fprintf (Fp, "#    with the terms of the license. Except as permitted by such\n");
        fprintf (Fp, "#    license, no part of this software or documentation may be\n");
        fprintf (Fp, "#    reproduced, stored in a retrieval system, or transmitted in any\n");
        fprintf (Fp, "#    form or by any means without the express written consent of\n");
        fprintf (Fp, "#    Intel Corporation.\n");
    }
    fprintf (Fp, "#\n");
    fprintf (Fp, "\n");
}

char*
LastName (
    IN char*    FullName
    )
{
    char       *p1, *p2;

    p2 = FullName;
    for (p1=FullName; *p1; p1++) {
        if (*p1 == '\\' || *p1 == '/') {
            p2 = p1 + 1;
        }
    }

    return p2;
}


VOID
ProcessMakeInfo (
    IN char*    DirName,
    IN UINTN    DirLevel
    )
{    
    FILE            *FpIn, *FpOut, *FpInc;
    BOOLEAN         Failed, f, RawMakefile;
    char           OutName[MAX_FILE_NAME];
    char           OutDir[MAX_FILE_NAME];
    char           IncName[MAX_FILE_NAME];
    char*           LName;
    MAKE_INFO       MakInfo;
    A_LINE          *Line;
    LIST_ENTRY     *Link;

    Failed = TRUE;
    FpIn = NULL;
    FpOut = NULL;
    FpInc = NULL;
    OutName[0] = 0;
    IncName[0] = 0;
    RawMakefile = FALSE;

    //
    // Create out directory
    //

    sprintf (OutDir, "%s\\output\\%s", BuildDir, DirName);
    mkdir (OutDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    //
    // Open input file
    //

    FpIn = fopen("make.inf", "r");
    if (!FpIn) {

        //
        // Check for makefile
        //

        FpIn = fopen("makefile", "r");
        if (!FpIn) {
            return ;
        }

        RawMakefile = TRUE;
    }

    printf ("Processing %s  ", DirName);
    InitMakeInfo (&MakInfo);

    //
    // Read input file
    //

    if (!RawMakefile) {
        f = ParseInput (FpIn, &MakInfo);
        if (f) {
            goto Done;
        }
    }

    //
    // Create output name
    //

    sprintf (OutName, "%s\\makefile", OutDir);
    FpOut = fopen (OutName, "w+");
    if (!FpOut) {
        printf ("Could not create %s\n", OutName);
        goto Done;
    }
    DumpHeader (FpOut, NULL);

    //
    // If the source is a raw makefile, just copy it
    //

    if (RawMakefile) {
        Failed = CopyFileData (FpIn, FpOut);
        goto Done;
    }

    sprintf (IncName, "%s\\makefile.hdr", OutDir);
    FpInc = fopen (IncName, "w+");
    if (!FpInc) {
        printf ("Could not create %s\n", IncName);
        goto Done;
    }
    DumpHeader (FpInc, NULL);

    //
    // Write header to output file
    // 

    DumpHeader (FpOut, "Globals");
    fprintf (FpOut, "SOURCE_DIR=$(EFI_SOURCE)\\%s\n", DirName);
    fprintf (FpOut, "BUILD_DIR=$(EFI_SOURCE)%s\\output\\%s\n", BuildPath, DirName);
    fprintf (FpOut, "\n");

    //
    // Include master.env
    //

    DumpHeader (FpOut, "Include Master.env enviroment");
    fprintf (FpOut, "!include $(EFI_SOURCE)%s\\master.env\n", BuildPath);

    //
    // Dump nmake section
    //

    if (!IsListEmpty (&MakInfo.NMake)) {
        DumpHeader (FpOut, "Gerneral make info");
        for (Link=MakInfo.NMake.Flink; Link != &MakInfo.NMake; Link = Link->Flink) {
            Line = _CR (Link, A_LINE, Link);
            fprintf (FpOut, "%s\n", Line->Line);
        }
    }

    //
    // If building for EBC, rename the entry point function to EfiMain so
    // that the startup EbcLib.lib entry point function EfiStart() knows what
    // to call.
    //
    fprintf (FpOut, "!IF \"$(PROCESSOR)\" == \"Ebc\"\n");
    fprintf (FpOut, "!IF DEFINED(IMAGE_ENTRY_POINT)\n");
    fprintf (FpOut, "!IF \"$(IMAGE_ENTRY_POINT)\" != \"EfiMain\"\n");
    fprintf (FpOut, "C_FLAGS = $(C_FLAGS) /D $(IMAGE_ENTRY_POINT)=EfiMain\n");
    fprintf (FpOut, "!ENDIF\n");
    fprintf (FpOut, "!ENDIF\n");
    fprintf (FpOut, "!ENDIF\n");

    //
    // Include header file includes
    //

    DumpHeader (FpOut, "Include paths");
    for (Link=MakInfo.Includes.Flink; Link != &MakInfo.Includes; Link = Link->Flink) {
        Line = _CR (Link, A_LINE, Link);
        if (strncmp(Line->Line, "$(EFI_SOURCE)\\", 14) == 0) {
            fprintf (FpOut, "!include $(EFI_SOURCE)%s\\output\\%s\\makefile.hdr\n", BuildPath, Line->Line+14);
            fprintf (FpOut, "INC=-I %s -I %s\\$(PROCESSOR) $(INC)\n", Line->Line, Line->Line);
        } else {
            fprintf (FpOut, "!include $(BUILD_DIR)\\%s\\makefile.hdr\n", Line->Line);
            fprintf (FpOut, "INC=-I $(SOURCE_DIR)\\%s -I $(SOURCE_DIR)\\%s\\$(PROCESSOR) $(INC)\n", 
                                Line->Line,
                                Line->Line
                                );
        }
        fprintf (FpOut, "\n");
    }

    //
    // Include libaries 
    //

    if (!IsListEmpty (&MakInfo.Libraries)) {
        DumpHeader (FpOut, "Libraries");
        for (Link=MakInfo.Libraries.Flink; Link != &MakInfo.Libraries; Link = Link->Flink) {
            Line = _CR (Link, A_LINE, Link);
            LName = LastName (Line->Line);
            if (strncmp(Line->Line, "$(EFI_SOURCE)\\", 14) == 0) {
                fprintf (FpOut, "LIBS = $(LIBS) $(EFI_SOURCE)%s\\output\\%s\\%s.lib\n", 
                                BuildPath, 
                                Line->Line+14,
                                LName
                                );
            } else {
                fprintf (FpOut, "LIBS = $(LIBS) $(SOURCE_DIR)\\%s\\%s.lib\n", 
                                Line->Line,
                                LName
                                );
            }
        }
    }

    //
    // Dump source file info
    //

    DumpHeader (FpOut, "Source file dependencies");
    // Dump common files
    DumpFileInfo (FpOut, &MakInfo.Comm);

    // Dump ia32 files    
    fprintf (FpOut, "!IF \"$(PROCESSOR)\" == \"Ia32\"\n");
    DumpFileInfo (FpOut, &MakInfo.Ia32);
    fprintf (FpOut, "!ENDIF\n\n");

    // Dump ipf files
    fprintf (FpOut, "!IF \"$(PROCESSOR)\" == \"Ipf\"\n");
    DumpFileInfo (FpOut, &MakInfo.Ipf);
    fprintf (FpOut, "!ENDIF\n\n");

    //
    // Dump ebc files.
    //
    fprintf (FpOut, "!IF \"$(PROCESSOR)\" == \"Ebc\"\n");
    DumpFileInfo (FpOut, &MakInfo.Ebc);
    fprintf (FpOut, "!ENDIF\n\n");

    //
    // Dump include file info
    //

    DumpIncludeInfo (FpInc, DirName, &MakInfo.Comm);

    // Dump ia32 files    
    fprintf (FpInc, "!IF \"$(PROCESSOR)\" == \"Ia32\"\n");
    DumpIncludeInfo(FpInc, DirName, &MakInfo.Ia32);
    fprintf (FpInc, "!ENDIF\n\n");

    // Dump ipf files
    fprintf (FpInc, "\n");
    fprintf (FpInc, "!IF \"$(PROCESSOR)\" == \"Ipf\"\n");
    DumpIncludeInfo (FpInc, DirName, &MakInfo.Ipf);
    fprintf (FpInc, "!ENDIF\n\n");

    // Dump ebc files
    fprintf (FpInc, "\n");
    fprintf (FpInc, "!IF \"$(PROCESSOR)\" == \"Ebc\"\n");
    DumpIncludeInfo (FpInc, DirName, &MakInfo.Ebc);
    fprintf (FpInc, "!ENDIF\n\n");

    DumpHeader (FpOut, "Define for EBC boot service drivers");
    //
    // !IF some code to the makefile for EBC linking. This is required
    // because the link command is different. Mainly the entry
    // point function has been renamed and fixed to be EfiStart().
    // Also make sure that they're not building a runtime driver.
    //
    fprintf (FpOut, "!IF \"$(PROCESSOR)\" == \"Ebc\"\n");
    fprintf (FpOut, "!IFDEF TARGET_BS_DRIVER\n");
    fprintf (FpOut, "TARGET_DRIVER = $(BIN_DIR)\\$(TARGET_BS_DRIVER).efi\n");
    fprintf (FpOut, "BIN_TARGETS = $(BIN_TARGETS) $(TARGET_DRIVER)\n");
    fprintf (FpOut, "$(TARGET_DRIVER) : $(OBJECTS) $(LIBS)\n");
    fprintf (FpOut, "    $(LINK) $(L_FLAGS) $(MODULE_LFLAGS) $** /ENTRY:EfiStart /OUT:$(@R).dll\n");
    fprintf (FpOut, "    $(FWIMAGE) bsdrv $(@R).dll $(TARGET_DRIVER)\n");
    fprintf (FpOut, "!ELSEIFDEF TARGET_RT_DRIVER\n");
    fprintf (FpOut, "!ERROR Runtime drivers are not supported for EBC\n");
    fprintf (FpOut, "!ENDIF\n");

    LName = LastName (DirName);

    DumpHeader (FpOut, "Define the lib");
    fprintf (FpOut, "!IFDEF OBJECTS\n");
    fprintf (FpOut, "TARGET_LIB = $(BUILD_DIR)\\%s.lib\n", LName);
    fprintf (FpOut, "BIN_TARGETS = $(BIN_TARGETS) $(TARGET_LIB)\n");
    fprintf (FpOut, "$(TARGET_LIB) : $(OBJECTS) \n");
    fprintf (FpOut, "    $(LIB) $(LIB_FLAGS) $** /OUT:$(TARGET_LIB)\n");
    fprintf (FpOut, "!ENDIF\n");

    fprintf (FpOut, "\n!ELSE\n\n");

    //
    // If there are any sources to build, define a lib target
    //

    LName = LastName (DirName);

    DumpHeader (FpOut, "Define the lib");
    fprintf (FpOut, "!IFDEF OBJECTS\n");
    fprintf (FpOut, "TARGET_LIB = $(BUILD_DIR)\\%s.lib\n", LName);
    fprintf (FpOut, "BIN_TARGETS = $(BIN_TARGETS) $(TARGET_LIB)\n");
    fprintf (FpOut, "$(TARGET_LIB) : $(OBJECTS) \n");
    fprintf (FpOut, "    $(LIB) $(LIB_FLAGS) $** /OUT:$(TARGET_LIB)\n");
    fprintf (FpOut, "!ENDIF\n");

    //
    //
    //

    DumpHeader (FpOut, "Define for apps");
    fprintf (FpOut, "!IFDEF TARGET_APP\n");
    fprintf (FpOut, "TARGET_APP = $(BIN_DIR)\\$(TARGET_APP).efi\n");
    fprintf (FpOut, "BIN_TARGETS = $(BIN_TARGETS) $(TARGET_APP)\n");
    fprintf (FpOut, "$(TARGET_APP) : $(TARGET_LIB) $(LIBS)\n");
    fprintf (FpOut, "    $(LINK) $(L_FLAGS) $(MODULE_LFLAGS) $** /ENTRY:$(IMAGE_ENTRY_POINT) /OUT:$(@R).dll\n");
    fprintf (FpOut, "    $(FWIMAGE) app $(@R).dll $(TARGET_APP)\n");
    fprintf (FpOut, "!ENDIF\n");
    
    //
    //
    //

    DumpHeader (FpOut, "Define for boot service drivers");
    fprintf (FpOut, "!IFDEF TARGET_BS_DRIVER\n");
    fprintf (FpOut, "TARGET_DRIVER = $(BIN_DIR)\\$(TARGET_BS_DRIVER).efi\n");
    fprintf (FpOut, "BIN_TARGETS = $(BIN_TARGETS) $(TARGET_DRIVER)\n");
    fprintf (FpOut, "$(TARGET_DRIVER) : $(TARGET_LIB) $(LIBS)\n");
    fprintf (FpOut, "    $(LINK) $(L_FLAGS) $(MODULE_LFLAGS) $** /ENTRY:$(IMAGE_ENTRY_POINT) /OUT:$(@R).dll\n");
    fprintf (FpOut, "    $(FWIMAGE) bsdrv $(@R).dll $(TARGET_DRIVER)\n");
    fprintf (FpOut, "!ENDIF\n");

    //
    // ENDIF of the EBC ELSE condition
    //
    fprintf (FpOut, "!ENDIF\n\n");
    //
    //
    //
    DumpHeader (FpOut, "Define for runtime service drivers");
    fprintf (FpOut, "!IFDEF TARGET_RT_DRIVER\n");
    fprintf (FpOut, "TARGET_DRIVER = $(BIN_DIR)\\$(TARGET_RT_DRIVER).efi\n");
    fprintf (FpOut, "BIN_TARGETS = $(BIN_TARGETS) $(TARGET_DRIVER)\n");
    fprintf (FpOut, "$(TARGET_DRIVER) : $(TARGET_LIB) $(LIBS)\n");
    fprintf (FpOut, "    $(LINK) $(L_FLAGS) $(MODULE_LFLAGS) $** /ENTRY:$(IMAGE_ENTRY_POINT) /OUT:$(@R).dll\n");
    fprintf (FpOut, "    $(FWIMAGE) rtdrv $(@R).dll $(TARGET_DRIVER)\n");
    fprintf (FpOut, "!ENDIF\n");

    //
    // Worked
    //

    DumpHeader (FpOut, "Handoff to Master.Mak");
    fprintf (FpOut, "!include $(EFI_SOURCE)%s\\master.mak\n", BuildPath);
    Failed = FALSE;
    printf ("\n");

Done:
    if (FpIn) {
        fclose (FpIn);
    }
    if (FpOut) {
        fclose (FpOut);
    }
    if (FpInc) {
        fclose (FpInc);
    }

    if (Failed) {
        if (OutName[0]) {
            unlink (OutName);
        }

        if (IncName[0]) {
            unlink (IncName);
        }
    }

    FreeMakeInfo (&MakInfo);
}
