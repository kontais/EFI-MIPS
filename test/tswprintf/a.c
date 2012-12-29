#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include "a.h"
typedef unsigned long long UINT64;
typedef unsigned int       UINTN;
typedef char               CHAR8;
typedef long long          INT64;

char    cs[10] = "a张";
wchar_t wb[20] = L"0123456789abcdef";
wchar_t ws[20] = L"a张";
wchar_t ws2[20] = L"b涛";



static wchar_t mHexStr[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
                            L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };



wchar_t *
AsciiToUnicode (
  CHAR8   *Ascii,
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
  INT64       Value, 
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

int main()
{
  printf("strlen(cs) = %d\n",strlen(cs));
  _swprintf(wb,12, L"%S1%S",ws,ws2);

  printf("cs[0] = 0x%x\n",cs[0]);
  printf("cs[1] = 0x%x\n",cs[1]);
  printf("cs[2] = 0x%x\n",cs[2]);
  printf("cs[3] = 0x%x\n",cs[3]);
  

  printf("wb[0] = 0x%x\n",wb[0]);
  printf("wb[1] = 0x%x\n",wb[1]);
  printf("wb[2] = 0x%x\n",wb[2]);
  printf("wb[3] = 0x%x\n",wb[3]);
  printf("wb[4] = 0x%x\n",wb[4]);
  printf("wb[5] = 0x%x\n",wb[5]);
  printf("wb[6] = 0x%x\n",wb[6]);
  printf("wb[7] = 0x%x\n",wb[7]);
  return 0;
}

