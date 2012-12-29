/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Str.c

Abstract:
    String operation functions



Revision History

--*/

#include "EfiShellLib.h"

INTN
StrCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2
  )
/*++

Routine Description:

  compare strings

Arguments:

    s1 - The first string
    s2 - The second string

Returns:

--*/
{
  ASSERT (s1 != NULL);
  ASSERT (s2 != NULL);
  
  while (*s1) {
    if (*s1 != *s2) {
      break;
    }

    s1 += 1;
    s2 += 1;
  }

  return *s1 -*s2;
}

INTN
StrnCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2,
  IN UINTN    len
  )
/*++

Routine Description:

  compare strings

Arguments:

    s1 - The first string
    s2 - The second string
    len - compary string length

Returns:

--*/
{
  
  ASSERT (s1 != NULL);
  ASSERT (s2 != NULL);
  
  while (*s1 && len) {
    if (*s1 != *s2) {
      break;
    }

    s1 += 1;
    s2 += 1;
    len -= 1;
  }

  return len ? *s1 -*s2 : 0;
}

VOID
StrLwr (
  IN CHAR16   *Str
  )
/*++

Routine Description:

  low case string

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UnicodeInterface->StrLwr (UnicodeInterface, Str);
}

VOID
StrUpr (
  IN CHAR16   *Str
  )
/*++

Routine Description:

  upper case string

Arguments:

  Str  -  The string
  
Returns:

--*/

{
  UnicodeInterface->StrUpr (UnicodeInterface, Str);
}

VOID
StrCpy (
  IN CHAR16   *Dest,
  IN CHAR16   *Src
  )
/*++

Routine Description:

  Copy string

Arguments:

  Dest  -  The dest string
  Src   -  The source string
  
Returns:

--*/
{
  ASSERT (Dest != NULL);
  ASSERT (Src != NULL);
  
  while (*Src) {
    *(Dest++) = *(Src++);
  }

  *Dest = 0;
}

VOID
StrCat (
  IN CHAR16   *Dest,
  IN CHAR16   *Src
  )
{
  ASSERT (Dest != NULL);
  ASSERT (Src != NULL);
  StrCpy (Dest + StrLen (Dest), Src);
}

UINTN
StrLen (
  IN CHAR16   *Str
  )
/*++

Routine Description:
  
  This function returns the number of Unicode characters in the Unicode string s1.
  
Arguments: 
  
  Str     - Pointer to the null-terminated string to get length            str will hold the trimmed string. 

  
Returns:

--*/
{
  UINTN len;

  ASSERT (Str != NULL);

  for (len = 0; *Str; Str += 1, len += 1)
    ;
  return len;
}

VOID
StrTrim (
  IN OUT CHAR16   *str,
  IN     CHAR16   c
  )
/*++

Routine Description:
  
  Removes (trims) specified leading and trailing characters from a string.
  
Arguments: 
  
  str     - Pointer to the null-terminated string to be trimmed. On return, 
            str will hold the trimmed string. 
  c       - Character will be trimmed from str.
  
Returns:

--*/
{
  CHAR16  *p1;

  CHAR16  *p2;
 
  ASSERT (str != NULL);

  if (*str == 0) {
    return ;
  }
  //
  // Trim off the leading characters c
  //
  for (p1 = str; *p1 && *p1 == c; p1++) {
    ;
  }

  p2 = str;
  if (p2 == p1) {
    while (*p1) {
      p2++;
      p1++;
    }
  } else {
    while (*p1) {
      *p2 = *p1;
      p1++;
      p2++;
    }

    *p2 = 0;
  }

  for (p1 = str + StrLen (str) - 1; p1 >= str && *p1 == c; p1--) {
    ;
  }

  if (p1 != str + StrLen (str) - 1) {
    *(p1 + 1) = 0;
  }
}

CHAR16 *
StrChr (
  IN  CHAR16  *Str,
  IN  CHAR16  c
  )
{
  ASSERT (Str != NULL);
  
  for (; *Str != c; ++Str) {
    if (*Str == '\0') {
      return NULL;
    }
  }

  return (CHAR16 *) Str;
}

UINTN
StrStr (
  IN  CHAR16  *Str,
  IN  CHAR16  *Pat
  )
{
  //
  // Name:
  //    StrStr -- Search Pat in Str
  // In:
  //    Str -- mother string
  //    Pat -- search pattern
  // Out:
  //     0 : not found
  //    >= 1 : found position + 1
  //
  INTN  *Failure;
  INTN  i;
  INTN  j;
  INTN  Lenp;
  INTN  Lens;

  ASSERT (Pat != NULL);
  ASSERT (Str != NULL);
  
  //
  // this function copies from some lib
  //
  Lenp  = StrLen (Pat);
  Lens  = StrLen (Str);

  if (0 == Lenp || 0 == Lens) {
    return 0;
  }

  Failure     = AllocatePool (Lenp * sizeof (INTN));
  Failure[0]  = -1;

  for (j = 1; j < Lenp; j++) {
    i = Failure[j - 1];
    while ((Pat[j] != Pat[i + 1]) && (i >= 0)) {
      i = Failure[i];
    }

    if (Pat[j] == Pat[i + 1]) {
      Failure[j] = i + 1;
    } else {
      Failure[j] = -1;
    }
  }

  i = 0;
  j = 0;
  while (i < Lens && j < Lenp) {
    if (Str[i] == Pat[j]) {
      i++;
      j++;
    } else if (j == 0) {
      i++;
    } else {
      j = Failure[j - 1] + 1;
    }
  }

  FreePool (Failure);

  //
  // 0: not found
  // >= 1: found position + 1
  //
  return ((j == Lenp) ? (i - Lenp) : -1) + 1;

}

UINTN
Xtoi (
  CHAR16  *str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  CHAR16  c;
  UINTN   m;
  
  ASSERT (str != NULL);
  
  m = (UINTN) -1 >> 4;
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // skip preceeding zeros
  //
  while (*str && *str == '0') {
    str += 1;
  }
  //
  // skip preceeding white space
  //
  if (*str && (*str == 'x' || *str == 'X')) {
    str += 1;
  }
  //
  // convert hex digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= 'a' && c <= 'f') {
      c -= 'a' - 'A';
    }

    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
      if (u > m) {
        return (UINTN) -1;
      }

      u = u << 4 | c - (c >= 'A' ? 'A' - 10 : '0');
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

UINTN
Atoi (
  CHAR16  *str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  CHAR16  c;
  UINTN   m;
  UINTN   n;
  
  ASSERT (str != NULL);
  
  m = (UINTN) -1 / 10;
  n = (UINTN) -1 % 10;
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // convert digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= '0' && c <= '9') {
      if (u > m || u == m && c - '0' > (INTN) n) {
        return (UINTN) -1;
      }

      u = (u * 10) + c - '0';
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

INTN
StriCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2
  )
/*++

Routine Description:

  Compary string

Arguments:

  s1   - The first string
  s2   - The second string
  
Returns:

--*/
{
  return UnicodeInterface->StriColl (UnicodeInterface, s1, s2);
}

BOOLEAN
EFIAPI
LibStubMetaiMatch (
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *String,
  IN CHAR16                           *Pattern
  )
{
  return MetaMatch (String, Pattern);
}

CHAR16
ToUpper (
  CHAR16  a
  )
{
  if ('a' <= a && a <= 'z') {
    return (CHAR16) (a - 0x20);
  } else {
    return a;
  }
}

CHAR16
ToLower (
  CHAR16  a
  )
{
  if ('A' <= a && a <= 'Z') {
    return (CHAR16) (a + 0x20);
  } else {
    return a;
  }
}

BOOLEAN
MetaMatch (
  IN CHAR16                           *String,
  IN CHAR16                           *Pattern
  )
/*++

Routine Description:
  Performs a case-insensitive comparison between a Null-terminated
  Unicode pattern string and a Null-terminated Unicode string. 
  The pattern string can use the '?' wildcard to match any character, 
  and the '*' wildcard to match any sub-string.

Arguments:
  String   - A pointer to a Null-terminated Unicode string.
  Pattern  - A pointer to a Null-terminated Unicode pattern string.

Returns:
  TRUE  - Pattern was found in String.
  FALSE - Pattern was not found in String.

--*/
{
  //
  // we use char '!' as our internal escape char
  //
  CHAR16  c;
  //
  CHAR16  p;
  CHAR16  *pb;
  CHAR16  *pe;
  CHAR16  *pb1;
  CHAR16  ch[3];
  BOOLEAN chf[3];

  BOOLEAN flag;
  BOOLEAN found;

  UINTN   i;

  ASSERT (String != NULL);
  ASSERT (Pattern != NULL);

  flag = FALSE;

  for (;;) {
    flag  = FALSE;
    p     = *Pattern;
    Pattern += 1;
    if (p == ESCAPE_CHAR) {
      p = *Pattern;
      Pattern += 1;
      flag = TRUE;
    }

    if (p == 0) {
      //
      // End of pattern.  If end of string, TRUE match
      //
      if (*String) {
        return FALSE;
      } else {
        return TRUE;
      }

    } else if (p == '*' && !flag) {
      //
      // Match zero or more chars
      //
      while (*String) {
        if (MetaMatch (String, Pattern)) {
          return TRUE;
        }

        String += 1;
      }

      return MetaMatch (String, Pattern);

    } else if (p == '?' && !flag) {
      //
      // Match any one char
      //
      if (!*String) {
        return FALSE;
      }

      String += 1;

    } else if (p == '[' && !flag) {
      c = *String;
      if (!c) {
        return FALSE;
        //
        // syntax problem
        //
      }

      pb    = Pattern;

      found = FALSE;
      for (;;) {
        flag  = FALSE;
        pe    = pb++;
        if (*pe == ESCAPE_CHAR) {
          flag  = TRUE;
          pe    = pb++;

        }

        if (*pe == L']' && !flag) {
          found = TRUE;
          pb    = Pattern;
          break;
        }
        //
        // Found ']' and
        // pe was point to ]
        //
        if (*pe == 0) {
          found = FALSE;
          break;
        }
        //
        // No ']' was found;
        //
      }

      if (!found) {
        if (c != L'[') {
          return FALSE;
          //
          // Do not match, return FALSE;
          //
        }
      } else {
        //
        // test two cases
        // [a-b]
        //
        pb1                   = pb;
        ch[0]                 = ch[1] = ch[2] = 0;

        for (i = 0; i < 3; i++) {
          chf[i]  = FALSE;
          ch[i]   = *pb++;
          if (*pb == ESCAPE_CHAR) {
            chf[i]  = TRUE;
            ch[i]   = *pb++;
          }

          if (pb == pe) {
            break;
          }
        }
        //
        // to test whether [char1 - char2] format.
        //
        if (ch[1] == L'-' && !chf[1] && pb == pe && i == 2) {
          if (!(ToUpper (c) >= ToUpper (ch[0]) && ToUpper (c) <= ToUpper (ch[2]))) {
            return FALSE;
            //
            // DON'T match
            //
          }
        }
        //
        // test failed, regard as [char1, char2, ,,, charN] format
        //
        else {
 
          while (pb1 < pe) {
            p = *pb1;
            if (p == ESCAPE_CHAR) {
              pb1++;
              p = *pb1;
            }
 
            if (p == c) {
              break;
            }
 
            pb1++;
 
          }
 
          if (pb1 == pe) {
            return FALSE;
 
          }
 
        }
 
        Pattern = pe + 1;
 
      }
 
      String += 1;

    } else {

      c = *String;
      if (ToUpper (c) != ToUpper (p)) {
        return FALSE;
      }

      String += 1;

    }

  }

}

INTN
EFIAPI
LibStubStriCmp (
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *s1,
  IN CHAR16                           *s2
  )
{
  ASSERT (s1 != NULL);
  ASSERT (s2 != NULL);
  
  while (*s1) {
    if (ToUpper(*s1) != ToUpper(*s2)) {
      break;
    }

    s1 += 1;
    s2 += 1;
  }

  return ToUpper(*s1) -ToUpper(*s2);
}

VOID
EFIAPI
LibStubStrUpr (
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *Str
  )
{
  for(;*Str != 0; Str ++) {
    *Str = ToUpper(*Str);
  }
}

VOID
EFIAPI
LibStubStrLwr(
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *Str
  )
{
  for(;*Str != 0; Str ++) {
    *Str = ToLower(*Str);
  }
}

UINTN
StrSize (
  IN CHAR16   *s1
  )
/*++

Routine Description:

  Get string size

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN len;

  if (!s1) {
    return 0;
  }

  for (len = 0; *s1; s1 += 1, len += 1);
  return (len + 1) * sizeof (CHAR16);
}

CHAR16 *
StrDuplicate (
  IN CHAR16   *Src
  )
/*++

Routine Description:

  Duplicate the string

Arguments:

  Src  -  The source string
  
Returns:

--*/
{
  CHAR16  *Dest;
  UINTN   Size;

  ASSERT (Src != NULL);

  Dest  = NULL;
  Size  = StrSize (Src);
  Dest  = AllocatePool (Size);
  if (Dest) {
    CopyMem (Dest, Src, Size);
  }

  return Dest;
}

UINTN
strlena (
  IN CHAR8    *s1
  )
/*++

Routine Description:

  Get string length

Arguments:

  s1  -  The string
  
Returns:

--*/
{
  UINTN len;

  ASSERT (s1 != NULL);

  for (len = 0; *s1; s1 += 1, len += 1);
  return len;
}

UINTN
strcmpa (
  IN CHAR8    *s1,
  IN CHAR8    *s2
  )
/*++

Routine Description:

  Compare ascii string

Arguments:

  s1  -  The first string
  s2  -  The second string
  
Returns:

--*/
{
  while (*s1) {
    if (*s1 != *s2) {
      break;
    }

    s1 += 1;
    s2 += 1;
  }

  return *s1 -*s2;
}

UINTN
strncmpa (
  IN CHAR8    *s1,
  IN CHAR8    *s2,
  IN UINTN    len
  )
/*++

Routine Description:

  compare ascii string

Arguments:

  s1  -  The first string
  s2  -  The second string
  len -  The compare string length
  
Returns:

--*/
{
  ASSERT (s1 != NULL);
  ASSERT (s2 != NULL);
  
  while (*s1 && len) {
    if (*s1 != *s2) {
      break;
    }

    s1 += 1;
    s2 += 1;
    len -= 1;
  }

  return len ? *s1 -*s2 : 0;
}

BOOLEAN
StrSubCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2,
  IN UINTN    len
  )
/*++

Routine Description:

  Performs a substring searching in a Null-terminated Unicode strings.
  
Arguments:
  s1  - Substring to search
  s2  - String to be searched in 
  len - The string length
  
Returns:

--*/
{
  CHAR16  c;
  UINTN   Result;
  
  ASSERT (s1 != NULL);
  ASSERT (s2 != NULL);
  c = 0;
  if (len < StrLen (s1)) {
    c       = s1[len];
    s1[len] = 0;
  }

  Result = StrStr (s2, s1);

  if (c != 0) {
    s1[len] = c;
  }

  return Result > 0 ? TRUE : FALSE;
}

UINT64
StrToUIntegerBase (
  IN  CHAR16      *Str,
  IN  UINTN       Base,
  OUT EFI_STATUS  *Status
  )
/*++

Routine Description:
  Transfer a string to an unsigned integer

Arguments:
  Str              Source string
  Base             Base
  Status           The status

Returns:

  The value about the string

--*/
{
  UINT64  Value;
  UINT64  NewValue;
  CHAR16  *StrTail;
  CHAR16  *p;
  CHAR16  c;
  UINTN   Len;

  Value   = 0;
  *Status = EFI_ABORTED;

  //
  // skip leading white space
  //
  while (*Str && *Str == ' ') {
    Str++;
  }
  //
  // For NULL Str, just return
  //
  if (!*Str) {
    return 0;
  }
  //
  // skip white space in tail
  //
  Len     = StrLen (Str);
  StrTail = Str + Len - 1;
  while (*StrTail == ' ') {
    *StrTail = 0;
    StrTail--;
  }

  Len = StrTail - Str + 1;

  p   = Str;
  for (; p <= StrTail; p++) {
    if ('0' <= *p && *p <= '9') {
      continue;
    }

    if (16 == Base) {
      if (('a' <= *p && *p <= 'f') || ('A' <= *p && *p <= 'F')) {
        continue;
      }
    }

    return 0;
  }
  //
  // Convert the string to value
  //
  for (; Str <= StrTail; Str++) {

    c = *Str;

    if (Base == 16) {
      if (RShiftU64 (Value, 60)) {
        //
        // Overflow here x16
        //
        return 0;
      }

      NewValue = LShiftU64 (Value, 4);
    } else {
      if (RShiftU64 (Value, 61)) {
        //
        // Overflow here x8
        //
        return 0;
      }

      NewValue  = LShiftU64 (Value, 3);
      Value     = LShiftU64 (Value, 1);
      NewValue += Value;
      if (NewValue < Value) {
        //
        // Overflow here
        //
        return 0;
      }
    }

    Value = NewValue;

    if (Base == 16 && c >= 'a' && c <= 'f') {
      c = (CHAR16) (c - 'a' + 'A');
    }

    if (Base == 16 && c >= 'A' && c <= 'F') {
      Value += (c - 'A') + 10;
    } else if (c >= '0' && c <= '9') {
      Value += (c - '0');
    } else {
      //
      // Unexpected char is encountered
      //
      return 0;
    }
  }

  *Status = EFI_SUCCESS;
  return Value;
}

UINT64
StrToUInteger (
  IN  CHAR16      *Str,
  OUT EFI_STATUS  *Status
  )
/*++

Routine Description:
  Transfer a string to an unsigned integer

Arguments:
  Str              Source string
  Status           The status

Returns:

  The value about the string

--*/
{
  UINT64  Value;
  UINT64  NewValue;
  CHAR16  *StrTail;
  CHAR16  c;
  UINTN   Base;
  UINTN   Len;

  Base    = 10;
  Value   = 0;
  *Status = EFI_ABORTED;

  //
  // skip leading white space
  //
  while (*Str && *Str == ' ') {
    Str++;
  }
  //
  // For NULL Str, just return
  //
  if (!*Str) {
    return 0;
  }
  //
  // skip white space in tail
  //
  Len     = StrLen (Str);
  StrTail = Str + Len - 1;
  while (*StrTail == ' ') {
    *StrTail = 0;
    StrTail--;
  }

  Len = StrTail - Str + 1;

  //
  // Check hex prefix '0x'
  //
  if (Len >= 2 && *Str == '0' && (*(Str + 1) == 'x' || *(Str + 1) == 'X')) {
    Str += 2;
    Len -= 2;
    Base = 16;
  }

  if (!Len) {
    return 0;
  }
  //
  // Convert the string to value
  //
  for (; Str <= StrTail; Str++) {

    c = *Str;

    if (Base == 16) {
      if (RShiftU64 (Value, 60)) {
        //
        // Overflow here x16
        //
        return 0;
      }

      NewValue = LShiftU64 (Value, 4);
    } else {
      if (RShiftU64 (Value, 61)) {
        //
        // Overflow here x8
        //
        return 0;
      }

      NewValue  = LShiftU64 (Value, 3);
      Value     = LShiftU64 (Value, 1);
      NewValue += Value;
      if (NewValue < Value) {
        //
        // Overflow here
        //
        return 0;
      }
    }

    Value = NewValue;

    if (Base == 16 && c >= 'a' && c <= 'f') {
      c = (CHAR16) (c - 'a' + 'A');
    }

    if (Base == 16 && c >= 'A' && c <= 'F') {
      Value += (c - 'A') + 10;
    } else if (c >= '0' && c <= '9') {
      Value += (c - '0');
    } else {
      //
      // Unexpected char is encountered
      //
      return 0;
    }
  }

  *Status = EFI_SUCCESS;
  return Value;
}

UINT64
StrToUInt (
  IN     CHAR16       *Str,
  IN     UINTN        Base,
  OUT    EFI_STATUS   *Status
  )
/*++

Routine Description:
  Transfer a string to an unsigned integer

Arguments:
  Str              Source string
  Base             Base
  Status           The status

Returns:

  The value about the string

--*/
{
  UINT64  Value;
  UINT64  NewValue;
  CHAR16  *StrTail;
  CHAR16  c;
  UINTN   Len;

  Value   = 0;
  *Status = EFI_ABORTED;

  if (!*Str) {
    return 0;
  }

  StrTrim (Str, L' ');

  Len     = StrLen (Str);
  StrTail = Str + Len - 1;
  //
  // Check hex prefix '0x'
  //
  if (Len >= 2 && *Str == '0' && (*(Str + 1) == 'x' || *(Str + 1) == 'X')) {
    Str += 2;
    Len -= 2;
    if (Base != 16) {
      *Status = EFI_ABORTED;
      return 0;
    }
  }

  if (!Len) {
    return 0;
  }
  //
  // Convert the string to value
  //
  for (; Str <= StrTail; Str++) {

    c = *Str;

    if (Base == 16) {
      if (RShiftU64 (Value, 60)) {
        //
        // Overflow here x16
        //
        return 0;
      }

      NewValue = LShiftU64 (Value, 4);
    } else {
      if (RShiftU64 (Value, 61)) {
        //
        // Overflow here x8
        //
        return 0;
      }

      NewValue  = LShiftU64 (Value, 3);
      Value     = LShiftU64 (Value, 1);
      NewValue += Value;
      if (NewValue < Value) {
        //
        // Overflow here
        //
        return 0;
      }
    }

    Value = NewValue;

    if (Base == 16 && c >= 'a' && c <= 'f') {
      c = (CHAR16) (c - 'a' + 'A');
    }

    if (Base == 16 && c >= 'A' && c <= 'F') {
      Value += (c - 'A') + 10;
    } else if (c >= '0' && c <= '9') {
      Value += (c - '0');
    } else {
      //
      // Unexpected char is encountered
      //
      return 0;
    }
  }

  *Status = EFI_SUCCESS;
  return Value;
}

BOOLEAN
MetaiMatch (
  IN CHAR16   *String,
  IN CHAR16   *Pattern
  )
{
  return UnicodeInterface->MetaiMatch (UnicodeInterface, String, Pattern);
}

VOID
StrTrimRight (
  IN OUT CHAR16   *str,
  IN     CHAR16   c
  )
/*++

Routine Description:
  
  Removes (trims) specified trailing characters from a string.
  
Arguments: 
  
  str     - Pointer to the null-terminated string to be trimmed. On return, 
            str will hold the trimmed string. 
  c       - Character will be trimmed from str.
  
Returns:

--*/
{
  CHAR16  *p1;

  if (*str == 0) {
    return ;
  }
  //
  // Trim off the trailing characters c
  //
  for (p1 = str + StrLen (str) - 1; p1 >= str && *p1 == c; p1--) {
    ;
  }

  if (p1 != str + StrLen (str) - 1) {
    *(p1 + 1) = 0;
  }
}

VOID
StrnCpy (
  OUT CHAR16                   *Dst,
  IN  CHAR16                   *Src,
  IN  UINTN                    Length
  )
/*++

Routine Description:
  Copy a string from source to destination

Arguments:
  Dst              Destination string
  Src              Source string
  Length           Length of destination string

Returns:

--*/
{
  UINTN SrcIndex;
  UINTN DstIndex;

  ASSERT (Dst != NULL);
  ASSERT (Src != NULL);

  SrcIndex = DstIndex = 0;
  while (DstIndex < Length) {
    Dst[DstIndex] = Src[SrcIndex];
    DstIndex++;
    SrcIndex++;
  }
  Dst[DstIndex] = 0;
}

BOOLEAN
HasWildcards (
  IN CHAR16 *Pattern
  )
{
  CHAR16  *Ptr;

  ASSERT (Pattern);

  for (Ptr = Pattern; *Ptr; Ptr++) {
    if ((*Ptr) == '*' || (*Ptr) == '?' || (*Ptr) == '[' || (*Ptr) == ']') {
      break;
    }
  }

  if (!(*Ptr)) {
    return FALSE;
  }

  return TRUE;
}
