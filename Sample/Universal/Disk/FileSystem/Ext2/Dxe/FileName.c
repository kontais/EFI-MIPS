/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FileName.c

Abstract:

  Ext2 file system init.

--*/

#include "Ext2.h"

CHAR16 *
Ext2GetNextNameComponent (
  IN  CHAR16      *Path,
  OUT CHAR16      *Name
  )
/*++

Routine Description:

  Takes Path as input, returns the next name component
  in Name, and returns the position after Name (e.g., the
  start of the next name component)

Arguments:

  Path                  - The path of one file.
  Name                  - The next name component in Path.

Returns:

  The position after Name in the Path

--*/
{
  while (*Path != 0 && *Path != PATH_NAME_SEPARATOR) {
    *Name++ = *Path++;
  }
  *Name = 0;
  //
  // Get off of trailing path name separator
  //
  while (*Path == PATH_NAME_SEPARATOR) {
    Path++;
  }

  return Path;
}

BOOLEAN
Ext2FileNameIsValid (
  IN  CHAR16  *InputFileName,
  OUT CHAR16  *OutputFileName
  )
/*++

Routine Description:

  Check whether the IFileName is valid long file name. If the IFileName is a valid
  long file name, then we trim the possible leading blanks and leading/trailing dots.
  the trimmed filename is stored in OutputFileName

Arguments:

  InputFileName         - The input file name.
  OutputFileName        - The output file name.


Returns:

  TRUE                  - The InputFileName is a valid long file name.
  FALSE                 - The InputFileName is not a valid long file name.

--*/
{
  CHAR16  *TempNamePointer;
  CHAR16  TempChar;
  //
  // Trim Leading blanks
  //
  while (*InputFileName == L' ') {
    InputFileName++;
  }

  TempNamePointer = OutputFileName;
  while (*InputFileName != 0) {
    *TempNamePointer++ = *InputFileName++;
  }
  //
  // Trim Trailing blanks and dots
  //
  while (TempNamePointer > OutputFileName) {
    TempChar = *(TempNamePointer - 1);
    if (TempChar != L' ' && TempChar != L'.') {
      break;
    }

    TempNamePointer--;
  }

  *TempNamePointer = 0;

  if (TempNamePointer - OutputFileName > EXT2_NAME_LEN) {
    return FALSE;
  }
  //
  // See if there is any illegal characters within the name
  //
  do {
    if (*OutputFileName < 0x20 ||
        *OutputFileName == '\"' ||
        *OutputFileName == '*' ||
        *OutputFileName == '/' ||
        *OutputFileName == ':' ||
        *OutputFileName == '<' ||
        *OutputFileName == '>' ||
        *OutputFileName == '?' ||
        *OutputFileName == '\\' ||
        *OutputFileName == '|'
        ) {
      return FALSE;
    }

    OutputFileName++;
  } while (*OutputFileName != 0);
  return TRUE;
}

