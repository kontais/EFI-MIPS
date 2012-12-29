/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  fwimage.c

Abstract:

  Converts a pe32/pe32+ image to an FW image type

--*/

//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "TianoCommon.h"
#include "EfiImage.h"
#include "EfiUtilityMsgs.c"
#include "SysLib.h"


#define UTILITY_NAME  "FwImage"

VOID
Usage (
  VOID
  )
{
  printf ("Usage: " UTILITY_NAME "  {-t time-date} {-e} {-r} [APPLICATION|BS_DRIVER|RT_DRIVER|SAL_RT_DRIVER|COMBINED_PEIM_DRIVER|SECURITY_CORE|PEI_CORE|PE32_PEIM|RELOCATABLE_PEIM] peimage [outimage]\n");
  printf ("  -t: Add Time Stamp for output image\n");
  printf ("  -e: Not clear ExceptionTable for output image\n");
  printf ("  -r: Not strip zero pending of .reloc for output image\n");
}

static
STATUS
FReadFile (
  FILE    *in,
  VOID    **Buffer,
  UINTN   *Length
  )
{
  fseek (in, 0, SEEK_END);
  *Length = ftell (in);
  *Buffer = malloc (*Length);
  fseek (in, 0, SEEK_SET);
  fread (*Buffer, *Length, 1, in);
  return STATUS_SUCCESS;
}

static
STATUS
FWriteFile (
  FILE    *out,
  VOID    *Buffer,
  UINTN   Length
  )
{
  fseek (out, 0, SEEK_SET);
  fwrite (Buffer, Length, 1, out);
  if ((ULONG) ftell (out) != Length) {
    Error (NULL, 0, 0, "write error", NULL);
    return STATUS_ERROR;
  }
  free (Buffer);
  return STATUS_SUCCESS;
}


#define FW_SECURITY_CORE  1

int
main (
  int  argc,
  char *argv[]
  )
/*++

Routine Description:

  Main function.

Arguments:

  argc - Number of command line parameters.
  argv - Array of pointers to command line parameter strings.

Returns:

  STATUS_SUCCESS - Utility exits successfully.
  STATUS_ERROR   - Some error occurred during execution.

--*/
{
  ULONG                        Type;
  PUCHAR                       Ext;
  PUCHAR                       p;
  PUCHAR                       pe;
  PUCHAR                       OutImageName;
  UCHAR                        outname[500];
  FILE                         *fpIn;
  FILE                         *fpOut;
  EFI_IMAGE_ELF_HEADERS        *ElfHdr;
  time_t                       TimeStamp;
  struct tm                    TimeStruct;
  BOOLEAN                      TimeStampPresent;
  BOOLEAN                      NeedClearExceptionTable;
  BOOLEAN                      NeedStripZeroPendingReloc;
  UINT8                        *FileBuffer;
  UINTN                        FileLength;
  UINTN                        ExtType;
  UINTN                        PadNum;

  SetUtilityName (UTILITY_NAME);
  //
  // Assign to fix compile warning
  //
  OutImageName      = NULL;
  Type              = 0;
  Ext               = 0;
  TimeStamp         = 0;
  TimeStampPresent  = FALSE;

  NeedClearExceptionTable   = TRUE;
  NeedStripZeroPendingReloc = TRUE;

  //
  // Look for -t time-date option first. If the time is "0", then
  // skip it.
  //
  if ((argc > 2) && !strcmp (argv[1], "-t")) {
    TimeStampPresent = TRUE;
    if (strcmp (argv[2], "0") != 0) {
      //
      // Convert the string to a value
      //
      memset ((char *) &TimeStruct, 0, sizeof (TimeStruct));
      if (sscanf(
          argv[2], "%d/%d/%d,%d:%d:%d",
          &TimeStruct.tm_mon,   /* months since January - [0,11] */
          &TimeStruct.tm_mday,  /* day of the month - [1,31] */
          &TimeStruct.tm_year,  /* years since 1900 */
          &TimeStruct.tm_hour,  /* hours since midnight - [0,23] */
          &TimeStruct.tm_min,   /* minutes after the hour - [0,59] */
          &TimeStruct.tm_sec    /* seconds after the minute - [0,59] */
            ) != 6) {
        Error (NULL, 0, 0, argv[2], "failed to convert to mm/dd/yyyy,hh:mm:ss format");
        return STATUS_ERROR;
      }
      //
      // Now fixup some of the fields
      //
      TimeStruct.tm_mon--;
      TimeStruct.tm_year -= 1900;
      //
      // Sanity-check values?
      // Convert
      //
      TimeStamp = mktime (&TimeStruct);
      if (TimeStamp == (time_t) - 1) {
        Error (NULL, 0, 0, argv[2], "failed to convert time");
        return STATUS_ERROR;
      }
    }
    //
    // Skip over the args
    //
    argc -= 2;
    argv += 2;
  }

  //
  // Look for -e option.
  //
  if ((argc > 1) && !strcmp (argv[1], "-e")) {
    NeedClearExceptionTable = FALSE;
    //
    // Skip over the args
    //
    argc -= 1;
    argv += 1;
  }

  //
  // Look for -r option
  //
  if ((argc > 1) && !strcmp (argv[1], "-r")) {
    NeedStripZeroPendingReloc = FALSE;
    //
    // Skip over the args
    //
    argc -= 1;
    argv += 1;
  }

  //
  // Check for enough args
  //
  if (argc < 3) {
    Usage ();
    return STATUS_ERROR;
  }

  if (argc == 4) {
    OutImageName = argv[3];
  }
  //
  // Get new image type
  //
  p = argv[1];
  if (*p == '/' || *p == '\\') {
    p += 1;
  }

  ExtType = 0;

  if (_stricmp (p, "app") == 0 || _stricmp (p, "APPLICATION") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION;
    Ext   = ".efi";

  } else if (_stricmp (p, "bsdrv") == 0 || _stricmp (p, "BS_DRIVER") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
    Ext   = ".efi";

  } else if (_stricmp (p, "rtdrv") == 0 || _stricmp (p, "RT_DRIVER") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER;
    Ext   = ".efi";

  } else if (_stricmp (p, "rtdrv") == 0 || _stricmp (p, "SAL_RT_DRIVER") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER;
    Ext   = ".efi";
  } else if (_stricmp (p, "SECURITY_CORE") == 0) {
    Type    = EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
    ExtType = FW_SECURITY_CORE;
    Ext     = ".sec";
  } else if (_stricmp (p, "peim") == 0 ||
           _stricmp (p, "PEI_CORE") == 0 ||
           _stricmp (p, "PE32_PEIM") == 0 ||
           _stricmp (p, "RELOCATABLE_PEIM") == 0 ||
           _stricmp (p, "combined_peim_driver") == 0
          ) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
    Ext   = ".pei";
  } else {
    Usage ();
    return STATUS_ERROR;
  }
  //
  // open source file
  //
  fpIn = fopen (argv[2], "rb");
  if (!fpIn) {
    Error (NULL, 0, 0, argv[2], "failed to open input file for reading");
    return STATUS_ERROR;
  }
  FReadFile (fpIn, (VOID **)&FileBuffer, &FileLength);

  //
  // Read the dos & pe hdrs of the image
  //
  ElfHdr = (EFI_IMAGE_ELF_HEADERS *) FileBuffer;
  if (*(UINT32*)(&ElfHdr->e_ident[0]) != EFI_IMAGE_ELF_SIGNATURE) {
    Error (NULL, 0, 0, argv[2], "ELF header signature not found in source image");
    fclose (fpIn);
    return STATUS_ERROR;
  }

  //
  // Set EFI_IMAGE_SUBSYSTEM, 使用ELF文件头的e_ident[12/13]来保留EFI映像类型
  //
  *(UINT16*)(&ElfHdr->e_ident[EI_SUBSYSTEM]) = (UINT16) Type;

  //
  // open output file
  //
  strcpy (outname, argv[2]);
  pe = NULL;
  for (p = outname; *p; p++) {
    if (*p == '.') {
      pe = p;
    }
  }

  if (!pe) {
    pe = p;
  }

  strcpy (pe, Ext);

  if (!OutImageName) {
    OutImageName = outname;
  }

  fpOut = fopen (OutImageName, "w+b");
  if (!fpOut) {
    Error (NULL, 0, 0, OutImageName, "could not open output file for writing");
    fclose (fpIn);
    return STATUS_ERROR;
  }

  FWriteFile (fpOut, FileBuffer, FileLength);

  //
  // SECURITY_CORE要求8字节对齐
  // elf + Pad -> efi 4字节对齐 + 4字节EFI_PE32_SECTION头 -> pe32 + 24字节FFS Hdr -> .sec 
  // 所以这里生成4字节对齐, 4 + 4 + 24 为8字节对齐
  //
  if (ExtType == FW_SECURITY_CORE) {
    PadNum = (FileLength + 4 + 24) % 8;
    while (PadNum % 8 != 0) {
     PadNum++;
     fputc(0x00,fpOut); 
    }
  }

  //
  // Done
  //
  fclose (fpIn);
  fclose (fpOut);

  return STATUS_SUCCESS;
}
