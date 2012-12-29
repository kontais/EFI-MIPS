#include "efi.h"
#include <dirent.h>
#include <sys/stat.h>
#include "syslib.h"

//
// implement WIN32 _spawnv
//
int _spawnv (INTN mode, char * file, char *argv[])
{
  pid_t child_pid;
  int WaitStatus;

  if (file == NULL) {
    return -1;
  }

  child_pid = fork();

  if (child_pid == -1) {
    return -1; // fork error
  }

  if (child_pid == 0) {
    return execvp((char*)file,(char**)argv);
  }
  else
  {
    if (mode & _P_WAIT) {
      if (child_pid == wait(&WaitStatus)) {
        if (WIFEXITED(WaitStatus)) {
          return WEXITSTATUS(WaitStatus);
        }
      }
    }
  }
  return -1; // execvp error
}

//
// implement _strlwr
//
void _strlwr (char* s)
{
  if (s == NULL) {
    return;
  }

  while (*s) {
    if ((*s >= 'A') && (*s <= 'Z')) {
      *s += 0x20;  // 'a' - 'A' = 0x20
    }
    s++;
  }
  return ;
}

UINTN GetFileSize(FILE *fp)
{
  UINTN FileSize;
  fpos_t PosBackup;
  fpos_t PosEnd;

  if (fp == NULL) {
    return -1;
  }

  //
  // backup the file postion & state
  //
  if (0 != fgetpos(fp,&PosBackup)) {
    return -1;
  }

  //
  // move file postion to end
  //
  if (0 != fseek(fp,0,SEEK_END)) {
    return -1;
  }

  // 
  // get postion and state
  //
  if (0 != fgetpos(fp,&PosEnd)) {
    return -1;
  }

  FileSize = _FPOSOFF(PosEnd);
  
  //
  // set postion & state back
  //
  if (0 != fsetpos(fp,&PosBackup)) {
    return -1;
  }

  return FileSize;
}

DIR *   FindFirstFile(char * Path, LIN32_FIND_DATA *FindData)
{
  DIR    *DirHandle = NULL;
  struct dirent * DirEntry;
  struct stat     FileStatus;
  char   FullName[_MAX_PATH];

  if (Path == NULL) {
    return DirHandle;
  }

  DirHandle = opendir(Path);
  if (DirHandle == NULL) {
    return DirHandle;
  }

  DirEntry = readdir(DirHandle);
  if (DirEntry == NULL) {
    closedir(DirHandle);
    DirHandle = NULL;
    return DirHandle;
  }

  strcpy(FindData->DirPath, Path);

  //
  // save file name
  //
  strcpy(FindData->cFileName, DirEntry->d_name);

  memset(FullName,0x00,_MAX_PATH);
  strcpy(FullName,FindData->DirPath);
  strcat(FullName,"/");
  strcat(FullName,DirEntry->d_name);

  if (stat(FullName, &FileStatus) == -1) {
    closedir(DirHandle);
    DirHandle = NULL;
    return DirHandle;
  }
  
  //
  // save file attribute
  //
  FindData->dwFileAttributes = FileStatus.st_mode;

  return DirHandle;
}

BOOLEAN FindNextFile(DIR *DirHandle, LIN32_FIND_DATA *FindData)
{
  struct dirent * DirEntry;
  struct stat     FileStatus;
  char   FullName[_MAX_PATH];

  if (DirHandle == NULL) {
    return FALSE;
  }

  DirEntry = readdir(DirHandle);

  if (DirEntry == NULL) {
    return FALSE;
  }

  //
  // save file name
  //
  strcpy(FindData->cFileName, DirEntry->d_name);

  memset(FullName,0x00,_MAX_PATH);
  strcpy(FullName,FindData->DirPath);
  strcat(FullName,"/");
  strcat(FullName,DirEntry->d_name);

  if (stat(FullName, &FileStatus) == -1) {
    return FALSE;
  }
  
  //
  // save file attribute
  //
  FindData->dwFileAttributes = FileStatus.st_mode;

  return TRUE;
}
void    FindClose(DIR *DirHandle)
{
  if (DirHandle == NULL) {
    return ;
  }

  closedir(DirHandle);
  DirHandle = NULL;

  return;
}

//
// libc wcscmp error if use -fshort-wchar and string not 4 byte align
//
int _wcscmp(wchar_t *s1, wchar_t *s2)
{
  while( *s1 && *s2 ) {
    if ( *s1 != *s2) {
      break;
    }
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

int _wcslen(wchar_t *s)
{
  int i = 0;
  while (*s) {
    s++;
    i++;
  }
  return i;
}

int _wcsncmp (wchar_t *s1, wchar_t *s2, size_t n)
{
  if (n == 0) {
    return n;
  }
  while ( *s1 && *s2 && (n != 0)) {
    if ( *s1 != *s2 ) {
      break;
    }
    if (--n == 0) {
      break;
    }
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

wchar_t *_wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
  wchar_t *const d = dest;
  if (n == 0) {
    return d;
  }
  while (*src && (n != 0)) {
    *dest++ = *src++;
    if (--n == 0) {
      break;
    }
  }
  *dest++ = L'\0';
  while (n-- != 0 ) {
    *dest++ = L'\0';
  }

  return d;
}

wchar_t *_wcscat(wchar_t *dest, const wchar_t *src)
{
  wchar_t *const d = dest;
  while (*dest) {
    dest++;
  }
  while(*src) {
    *dest++ = *src++;
  }
  *dest = L'\0';

  return d;
}

wchar_t *_wcscpy(wchar_t *dest, wchar_t *src)
{
  wchar_t *const d = dest;
  while (*src) {
    *dest++ = *src++;
  }
  *dest = L'\0';

  return d;
}



static wchar_t mHexStr[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
                            L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };

char *
UnicodeToAscii (
  wchar_t   *Unicode
)
/*++

Routine Description:
  Convert the passed in Ascii string to Unicode.
  Optionally return the length of the strings.

Arguments:
  Ascii   - Ascii string to convert
  StrLen  - Length of string 

Returns:
  Pointer to malloc'ed Unicode version of Ascii

注意:
  函数内部没有释放申请的空间

--*/
{
  UINTN   Index;
  char  *Ascii;
  UINTN  Len;

  //
  // Allocate a buffer for ascii string
  //
  Len = _wcslen(Unicode);
  
  Ascii = malloc (Len * sizeof (wchar_t) + 1);
  if (Ascii == NULL) {
    return NULL;
  }

  for (Index = 0; Index < Len; Index++) {
    Ascii[Index] = (char) Unicode[Index];
  }

  Ascii[Index] = '\0';


  return Ascii;
}


wchar_t *
AsciiToUnicode (
  char   *Ascii,
  UINTN   *StrLen
  )
/*++

Routine Description:
  Convert the passed in Ascii string to Unicode.
  Optionally return the length of the strings.

Arguments:
  Ascii   - Ascii string to convert
  StrLen  - Length of string 

Returns:
  Pointer to malloc'ed Unicode version of Ascii

注意:
  函数内部没有释放申请的空间

--*/
{
  UINTN   Index;
  wchar_t  *Unicode;

  //
  // Allocate a buffer for unicode string
  //
  for (Index = 0; Ascii[Index] != '\0'; Index++)
    ;
  Unicode = malloc ((Index + 1) * sizeof (wchar_t));
  if (Unicode == NULL) {
    return NULL;
  }

  for (Index = 0; Ascii[Index] != '\0'; Index++) {
    Unicode[Index] = (wchar_t) Ascii[Index];
  }

  Unicode[Index] = '\0';

  if (StrLen != NULL) {
    *StrLen = Index;
  }

  return Unicode;
}



UINT64
DivU64x32 (
  UINT64   Dividend,
  UINTN    Divisor,
  UINTN   *Remainder
  )
/*++

Routine Description:

  This routine allows a 64 bit value to be divided with a 32 bit value returns 
  64bit result and the Remainder.

Arguments:

  Dividend  - dividend
  Divisor   - divisor
  Remainder - buffer for remainder
 
Returns:

  Dividend  / Divisor
  Remainder = Dividend mod Divisor

--*/
{
  if (Remainder != NULL) {
    *Remainder = Dividend % Divisor;
  }

  return Dividend / Divisor;
}



UINT64
RShiftU64 (
  UINT64   Operand,
  UINTN    Count
  )
/*++

Routine Description:

  This routine allows a 64 bit value to be right shifted by 32 bits and returns the 
  shifted value.
  Count is valid up 63. (Only Bits 0-5 is valid for Count)

Arguments:

  Operand - Value to be shifted
  Count   - Number of times to shift right.
 
Returns:

  Value shifted right identified by the Count.

--*/
{
  return Operand >> Count;
}


UINTN
EfiValueToHexStr (
  wchar_t  *Buffer, 
  UINT64      Value, 
  UINTN       Flags, 
  UINTN       Width
  )
/*++

Routine Description:

  VSPrint worker function that prints a Value as a hex number in Buffer

Arguments:

  Buffer - Location to place ascii hex string of Value.

  Value  - Hex value to convert to a string in Buffer.

  Flags  - Flags to use in printing Hex string, see file header for details.

  Width  - Width of hex value.

Returns: 

  Number of characters printed.  

--*/
{
  wchar_t  TempBuffer[CHARACTER_NUMBER_FOR_VALUE];
  wchar_t  *TempStr;
  wchar_t  Prefix;
  wchar_t  *BufferPtr;
  UINTN   Count;
  UINTN   Index;

  TempStr   = TempBuffer;
  BufferPtr = Buffer;

  //
  // Count starts at one since we will null terminate. Each iteration of the
  // loop picks off one nibble. Oh yea TempStr ends up backwards
  //
  Count = 0;
  
  if (Width > CHARACTER_NUMBER_FOR_VALUE - 1) {
    Width = CHARACTER_NUMBER_FOR_VALUE - 1;
  }

  do {
    Index = ((UINTN)Value & 0xf);
    *(TempStr++) = mHexStr[Index];
    Value = RShiftU64 (Value, 4);
    Count++;
  } while (Value != 0);

  if (Flags & PREFIX_ZERO) {
    Prefix = '0';
  } else { 
    Prefix = ' ';
  }

  Index = Count;
  if (!(Flags & LEFT_JUSTIFY)) {
    for (; Index < Width; Index++) {
      *(TempStr++) = Prefix;
    }
  }

  //
  // Reverse temp string into Buffer.
  //
  if (Width > 0 && (UINTN) (TempStr - TempBuffer) > Width) {
    TempStr = TempBuffer + Width;
  }
  Index = 0;
  while (TempStr != TempBuffer) {
    *(BufferPtr++) = *(--TempStr);
    Index++;
  }
    
  *BufferPtr = 0;
  return Index;
}


UINTN
EfiValueToString (
  wchar_t  *Buffer, 
  long long       Value, 
  UINTN       Flags,
  UINTN       Width
  )
/*++

Routine Description:

  VSPrint worker function that prints a Value as a decimal number in Buffer

Arguments:

  Buffer - Location to place ascii decimal number string of Value.

  Value  - Decimal value to convert to a string in Buffer.

  Flags  - Flags to use in printing decimal string, see file header for details.

  Width  - Width of hex value.

Returns: 

  Number of characters printed.  

--*/
{
  wchar_t    TempBuffer[CHARACTER_NUMBER_FOR_VALUE];
  wchar_t    *TempStr;
  wchar_t    *BufferPtr;
  UINTN     Count;
  UINTN     ValueCharNum;
  UINTN     Remainder;
  wchar_t    Prefix;
  UINTN     Index;
  int   ValueIsNegative;

  TempStr         = TempBuffer;
  BufferPtr       = Buffer;
  Count           = 0;
  ValueCharNum    = 0;
  ValueIsNegative = FALSE;

  if (Width > CHARACTER_NUMBER_FOR_VALUE - 1) {
    Width = CHARACTER_NUMBER_FOR_VALUE - 1;
  }

  if (Value < 0) {
    Value           = -Value;
    ValueIsNegative = TRUE;
  }

  do {
    Value = (long long)DivU64x32 ((UINT64)Value, 10, &Remainder);
    *(TempStr++) = (wchar_t)(Remainder + '0');
    ValueCharNum++;
    Count++;
    if ((Flags & COMMA_TYPE) == COMMA_TYPE) {
      if (ValueCharNum % 3 == 0 && Value != 0) {
        *(TempStr++) = ',';
        Count++;
      }
    }
  } while (Value != 0);

  if (ValueIsNegative) {
    *(TempStr++)    = '-';
    Count++;
  }

  if ((Flags & PREFIX_ZERO) && !ValueIsNegative) {
    Prefix = '0';
  } else { 
    Prefix = ' ';
  }                   

  Index = Count;
  if (!(Flags & LEFT_JUSTIFY)) {
    for (; Index < Width; Index++) {
      *(TempStr++) = Prefix;
    }
  }

  //
  // Reverse temp string into Buffer.
  //
  if (Width > 0 && (UINTN) (TempStr - TempBuffer) > Width) {
    TempStr = TempBuffer + Width;
  }
  Index = 0;
  while (TempStr != TempBuffer) {
    *(BufferPtr++) = *(--TempStr);
    Index++;
  }

  *BufferPtr = 0;
  return Index;
}


static
wchar_t *
GetFlagsAndWidth (
  wchar_t      *Format,
  UINTN       *Flags,
  UINTN       *Width,
  va_list *Marker
  )
/*++

Routine Description:

  VSPrint worker function that parses flag and width information from the 
  Format string and returns the next index into the Format string that needs
  to be parsed. See file headed for details of Flag and Width.

Arguments:

  Format - Current location in the VSPrint format string.

  Flags  - Returns flags

  Width  - Returns width of element

  Marker - Vararg list that may be paritally consumed and returned.

Returns: 

  Pointer indexed into the Format string for all the information parsed
  by this routine.

--*/
{
  UINTN   Count;
  int Done;

  *Flags  = 0;
  *Width  = 0;
  for (Done = FALSE; !Done;) {
    Format++;

    switch (*Format) {

    case '-':
      *Flags |= LEFT_JUSTIFY;
      break;

    case '+':
      *Flags |= PREFIX_SIGN;
      break;

    case ' ':
      *Flags |= PREFIX_BLANK;
      break;

    case ',':
      *Flags |= COMMA_TYPE;
      break;

    case 'L':
    case 'l':
      *Flags |= LONG_TYPE;
      break;

    case '*':
      *Width = va_arg (*Marker, UINTN);
      break;

    case '0':
      *Flags |= PREFIX_ZERO;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      Count = 0;
      do {
        Count = (Count * 10) +*Format - '0';
        Format++;
      } while ((*Format >= '0') && (*Format <= '9'));
      Format--;
      *Width = Count;
      break;

    default:
      Done = TRUE;
    }
  }

  return Format;
}

/*
  %[flags][width]type

  flags:
    '-' - Left justify
    '+' - Prefix a sign
    ' ' - Prefix a blank
    ',' - Place commas in numberss
    '0' - Prefix for width with zeros
    'l' - unsigned long long
    'L' - unsigned long long

  width:
    '*' - Get width from a unsigned int argumnet from the argument list
    Decimal number that represents width of print

  type:
    'X' - argument is a unsigned int hex number, prefix '0'
    'x' - argument is a hex number
    'd' - argument is a decimal number
    'a' - argument is an ascii string 
    'S','s' - argument is an Unicode string
    'g' - argument is a pointer to an EFI_GUID
    't' - argument is a pointer to an EFI_TIME structure
    'c' - argument is an ascii character
    'r' - argument is EFI_STATUS
    '%' - Print a %

*/

int _vswprintf(wchar_t *StartOfBuffer, size_t BufferSize, const wchar_t *FormatString, va_list Marker)
{
  wchar_t    TempBuffer[CHARACTER_NUMBER_FOR_VALUE];
  wchar_t    *Buffer;
  unsigned char     *AsciiStr;
  wchar_t    *UnicodeStr;
  wchar_t    *Format;
  UINTN     Index;
  UINTN     Flags;
  UINTN     Width;
  UINTN     Count;
  UINTN     NumberOfCharacters;
  UINTN     BufferLeft;
  UINT64    Value;

  //
  // Process the format string. Stop if Buffer is over run.
  //

  Buffer              = StartOfBuffer;
  Format              = (wchar_t *) FormatString;
  NumberOfCharacters  = BufferSize / sizeof (wchar_t);
  BufferLeft          = BufferSize;
  for (Index = 0; (*Format != '\0') && (Index < NumberOfCharacters - 1); Format++) {
    if (*Format != '%') {
      if ((*Format == '\n') && (Index < NumberOfCharacters - 2)) {
        //
        // If carage return add line feed
        //
        Buffer[Index++] = '\r';
        BufferLeft -= sizeof (wchar_t);
      }

      Buffer[Index++] = *Format;
      BufferLeft -= sizeof (wchar_t);
    } else {
      
      //
      // Now it's time to parse what follows after %
      //
      Format = GetFlagsAndWidth (Format, &Flags, &Width, &Marker);
      switch (*Format) {
      case 'X':
        Flags |= PREFIX_ZERO;
        Width = sizeof (UINT64) * 2;

      //
      // break skiped on purpose
      //
      case 'x':
        if ((Flags & LONG_TYPE) == LONG_TYPE) {
          Value = va_arg (Marker, UINT64);
        } else {
          Value = va_arg (Marker, UINTN);
        }

        EfiValueToHexStr (TempBuffer, Value, Flags, Width);
        UnicodeStr = TempBuffer;

        for (; (*UnicodeStr != '\0') && (Index < NumberOfCharacters - 1); UnicodeStr++) {
          Buffer[Index++] = *UnicodeStr;
        }
        break;

      case 'd':
        if ((Flags & LONG_TYPE) == LONG_TYPE) {
          Value = va_arg (Marker, UINT64);
        } else {
          Value = (UINTN) va_arg (Marker, UINTN);
        }

        EfiValueToString (TempBuffer, Value, Flags, Width);
        UnicodeStr = TempBuffer;

        for (; (*UnicodeStr != '\0') && (Index < NumberOfCharacters - 1); UnicodeStr++) {
          Buffer[Index++] = *UnicodeStr;
        }
        break;

      case 's':
      case 'S':
        UnicodeStr = (wchar_t *) va_arg (Marker, wchar_t *);
        if (UnicodeStr == NULL) {
          UnicodeStr = L"<null string>";
        }

        for (Count = 0; (*UnicodeStr != '\0') && (Index < NumberOfCharacters - 1); UnicodeStr++, Count++) {
          Buffer[Index++] = *UnicodeStr;
        }
        //
        // Add padding if needed
        //
        for (; (Count < Width) && (Index < NumberOfCharacters - 1); Count++) {
          Buffer[Index++] = ' ';
        }

        break;

      case 'a':
        AsciiStr = (unsigned char *) va_arg (Marker, unsigned char *);
        if (AsciiStr == NULL) {
          AsciiStr = "<null string>";
        }

        for (Count = 0; (*AsciiStr != '\0') && (Index < NumberOfCharacters - 1); AsciiStr++, Count++) {
          Buffer[Index++] = (wchar_t) * AsciiStr;
        }
        //
        // Add padding if needed
        //
        for (; (Count < Width) && (Index < NumberOfCharacters - 1); Count++) {
          Buffer[Index++] = ' ';
        }
        break;

      case 'c':
        Buffer[Index++] = (wchar_t) va_arg (Marker, UINTN);
        break;

      case '%':
        Buffer[Index++] = *Format;
        break;

      default:
        //
        // if the type is unknown print it to the screen
        //
        Buffer[Index++] = *Format;
      }

      BufferLeft = BufferSize - Index * sizeof (wchar_t);
    }
  }

  Buffer[Index++] = '\0';

  return &Buffer[Index] - StartOfBuffer;
}

int _swprintf(wchar_t * Buffer, size_t BufferSize, const wchar_t *Format, ...)
{
  UINTN Return;
  va_list Marker;

  va_start (Marker, Format);
  Return = _vswprintf(Buffer, BufferSize, Format, Marker);
  va_end (Marker);

  return Return;
}
