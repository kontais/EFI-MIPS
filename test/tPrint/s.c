#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EfiCommon.h"

#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)



UINT32 Trace = 0;
UINT32 Index = 0;
UINT32 Data[100];

EFI_SIMPLE_TEXT_OUT_PROTOCOL printfOut;

VOID
FreePool (
  IN VOID                 *Buffer
  );

//
// Math Functions
//
UINT64
LShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  );

UINT64
RShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  );

UINT64
MultU64x32 (
  IN UINT64   Multiplicand,
  IN UINTN    Multiplier
  );

UINT64
DivU64x32 (
  IN UINT64   Dividend,
  IN UINTN    Divisor,
  OUT UINTN   *Remainder OPTIONAL
  );




#define PRINT_STRING_LEN        1024
#define PRINT_ITEM_BUFFER_LEN   100
#define PRINT_JOINT_BUFFER_LEN  4

typedef struct {
  BOOLEAN Ascii;
  UINTN   Index;
  union {
    CHAR16  *pw;
    CHAR8   *pc;
  } u;
} POINTER;

typedef struct _pitem {

  POINTER Item;
  CHAR16  *Scratch;
  UINTN   Width;
  UINTN   FieldWidth;
  UINTN   *WidthParse;
  CHAR16  Pad;
  BOOLEAN PadBefore;
  BOOLEAN Comma;
  BOOLEAN Long;
} PRINT_ITEM;

typedef struct _pstate {
  //
  // Input
  //
  POINTER fmt;
  VA_LIST args;

  //
  // Output
  //
  CHAR16  *Buffer;
  CHAR16  *End;
  CHAR16  *Pos;
  UINTN   Len;

  UINTN   Attr;
  UINTN   RestoreAttr;

  UINTN   AttrNorm;
  UINTN   AttrHighlight;
  UINTN   AttrError;
  UINTN   AttrBlueColor;
  UINTN   AttrGreenColor;

  INTN (*Output) (VOID *context, CHAR16 *str);
  INTN (*SetAttr) (VOID *context, UINTN attr);
  VOID          *Context;

  //
  // Current item being formatted
  //
  struct _pitem *Item;
} PRINT_STATE;

typedef struct {
  BOOLEAN PageBreak;
  BOOLEAN AutoWrap;
  INTN    MaxRow;
  INTN    MaxColumn;
  INTN    InitRow;
  INTN    Row;
  INTN    Column;
  BOOLEAN OmitPrint;
  BOOLEAN OutputPause;
} PRINT_MODE;

PRINT_MODE mPrintMode;

//
// Internal fucntions
//
UINTN
_Print (
  IN PRINT_STATE     *ps
  );

INTN
_SPrint (
  IN VOID     *Context,
  IN CHAR16   *Buffer
  );

UINTN
_IPrint (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *Out,
  IN CHAR16                           *fmt,
  IN VA_LIST                          args
  );

VOID
PFLUSH (
  IN OUT PRINT_STATE     *ps
  );

VOID
PPUTC (
  IN OUT PRINT_STATE     *ps,
  IN CHAR16              c
  );

CHAR16
PGETC (
  IN POINTER      *p
  );

VOID
PITEM (
  IN OUT PRINT_STATE  *ps
  );



EFI_STATUS
EFIAPI
printfOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  WString - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  CHAR16 *p;
  p = WString;
  while (*p) {
    printf("%c", (CHAR8) *p);
    p++;
  }

}



VOID
SetMem (
  IN VOID     *Buffer,
  IN UINTN    Size,
  IN UINT8    Value
  )
/*++

Routine Description:
  Fills a buffer with a value.

Arguments:
  Buffer                - A pointer to the buffer to free.

  Size                  - The number of bytes in the buffer to fill.

  Value                 - The value to fill Buffer with.

Returns:

  None

--*/
{
  volatile INT8  *pt;


  pt = Buffer;
  while (Size--) {
    *(pt++) = Value;
  }
}

VOID
CopyMem (
  IN VOID     *Dest,
  IN VOID     *Src,
  IN UINTN    len
  )
/*++

Routine Description:
  Copies the contents of one buffer to another.

Arguments:
  Dest                - A pointer to the buffer to copy to

  Src                 - A pointer to the buffer to copy from.

  len                 - The number of bytes to copy.

Returns:

  None

--*/
{
  CHAR8 *Destination8;
  CHAR8 *Source8;


  Destination8  = Dest;
  Source8       = Src;

  if (Destination8 > Source8 && (Source8 + len - 1) > Destination8) {
    Source8 += (len - 1);
    Destination8 += (len - 1);
    while (len--) {
      *(Destination8--) = *(Source8--);
    }

    return ;
  }

  while (len--) {
    *(Destination8++) = *(Source8++);
  }
}

VOID *
AllocatePool (
  IN UINTN                Size
  )
/*++

Routine Description:
  Allocates pool memory.

Arguments:
  Size                 - Size in bytes of the pool being requested.

Returns:

  EFI_SUCEESS          - The requested number of bytes were allocated.

  EFI_OUT_OF_RESOURCES - The pool requested could not be allocated.

--*/
{
  VOID        *p;

  p = malloc(Size);

  return p;
}

VOID *
ReallocatePool (
  IN VOID                 *OldPool,
  IN UINTN                OldSize,
  IN UINTN                NewSize
  )
/*++

Routine Description:
  Adjusts the size of a previously allocated buffer.

Arguments:
  OldPool               - A pointer to the buffer whose size is being adjusted.
  OldSize               - The size of the current buffer.
  NewSize               - The size of the new buffer.

Returns:

  EFI_SUCEESS           - The requested number of bytes were allocated.

  EFI_OUT_OF_RESOURCES  - The pool requested could not be allocated.

  EFI_INVALID_PARAMETER - The buffer was invalid.

--*/
{
  VOID  *NewPool;

  NewPool = NULL;
  if (NewSize) {
    NewPool = AllocatePool (NewSize);
  }

  if (OldPool) {
    if (NewPool) {
      CopyMem (NewPool, OldPool, OldSize < NewSize ? OldSize : NewSize);
    }

    FreePool (OldPool);
  }

  return NewPool;
}

VOID *
AllocateZeroPool (
  IN UINTN                Size
  )
/*++

Routine Description:
  Allocates pool memory and initializes the memory to zeros.

Arguments:
  Size                 - Size in bytes of the pool being requested.

Returns:

  EFI_SUCEESS          - The requested number of bytes were allocated.

  EFI_OUT_OF_RESOURCES - The pool requested could not be allocated.

--*/
{
  VOID  *p;

  p = AllocatePool (Size);
  if (p) {
    SetMem (p, Size, 0);
  }

  return p;
}

INTN
CompareMem (
  IN VOID     *Dest,
  IN VOID     *Src,
  IN UINTN    len
  )
/*++

Routine Description:
  Compares the contents of one buffer to another.

Arguments:
  Dest                - A pointer to the buffer to compare

  Src                 - A pointer to the buffer to compare

  len                 - The number of bytes to compare

Returns:

  0                   - Dest is identical to Src for len bytes.
  !=0                 - Dest is not identical to Src for len bytes.

--*/
{
  CHAR8 *d;

  CHAR8 *s;


  d = Dest;
  s = Src;
  while (len--) {
    if (*d != *s) {
      return *d -*s;
    }

    d += 1;
    s += 1;
  }

  return 0;
}

VOID
ZeroMem (
  IN VOID     *Buffer,
  IN UINTN    Size
  )
/*++

Routine Description:
  Fills a buffer with zeros.

Arguments:
  Buffer                - A pointer to the buffer to fill with zeros.

  Size                  - Number of bytes to zero in the buffer.

Returns:

  None

--*/
{
  SetMem (Buffer, Size, 0);
}

VOID
FreePool (
  IN VOID                 *Buffer
  )
/*++

Routine Description:
  Releases a previously allocated buffer.

Arguments:
  Buffer                - A pointer to the buffer to free.

Returns:

  EFI_SUCEESS           - The requested number of bytes were allocated.

  EFI_INVALID_PARAMETER - The buffer was invalid.

--*/
{
  free (Buffer);
}












UINTN
Print (
  IN CHAR16     *fmt,
  ...
  )
/*++

Routine Description:

  Prints a formatted unicode string to the default console

Arguments:

  fmt         - Format string

Returns:

  Length of string printed to the console

--*/
{
  VA_LIST args;

  VA_START (args, fmt);
  return _IPrint (&printfOut, fmt, args);
}


UINTN
_IPrint (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *Out,
  IN CHAR16                           *fmt,
  IN VA_LIST                          args
  )
/*++
Routine Description:

  Display string worker for: Print, PrintAt, IPrint, IPrintAt

Arguments:

    Column - Column
    Row    - Row 
    Out    - Out 
    fmt    - fmt 
    fmta   - fmta
    args   - args

Returns:


--*/
{
  PRINT_STATE ps;



  SetMem (&ps, sizeof (ps), 0);
  ps.Context  = Out;
  ps.Output   = (INTN (*) (VOID *, CHAR16 *)) Out->OutputString;

  ps.fmt.u.pw = fmt;

  ps.args = args;


  return _Print (&ps);
}


VOID
PFLUSH (
  IN OUT PRINT_STATE     *ps
  )
{

  EFI_STATUS    Status;

  *ps->Pos = 0;

  ps->Output (ps->Context, ps->Buffer);
  

  CopyMem (
    ((CHAR8 *) (ps->Buffer)) - PRINT_JOINT_BUFFER_LEN,
    ((CHAR8 *) (ps->Pos)) - PRINT_JOINT_BUFFER_LEN,
    PRINT_JOINT_BUFFER_LEN
    );
  ps->Pos = ps->Buffer;
}


void
PSETATTR (
  IN OUT PRINT_STATE    *ps,
  IN UINTN              Attr
  )
{
  PFLUSH (ps);

  ps->RestoreAttr = ps->Attr;
  if (ps->SetAttr) {
    ps->SetAttr (ps->Context, Attr);
  }

  ps->Attr = Attr;
}

void
PPUTC (
  IN OUT PRINT_STATE     *ps,
  IN CHAR16              c
  )
{
  //
  // If Omit print to ConOut, then return.
  //
  //
  // if this is a newline and carriage return does not exist,
  // add a carriage return
  //
  if (c == '\n' && ps->Pos >= ps->Buffer && (CHAR16) *(ps->Pos - 1) != '\r') {
    PPUTC (ps, '\r');
  }

  *ps->Pos = c;
  ps->Pos += 1;
  ps->Len += 1;

  //
  // if at the end of the buffer, flush it
  //
  if (ps->Pos >= ps->End) {
    PFLUSH (ps);
  }
}

CHAR16
PGETC (
  IN POINTER      *p
  )
{
  CHAR16  c;


  c = (CHAR16) (p->u.pw[p->Index]);
  p->Index += 1;

  return c;
}

void
PITEM (
  IN OUT PRINT_STATE  *ps
  )
{
  UINTN       Len;

  UINTN       i;
  PRINT_ITEM  *Item;
  CHAR16      c;


  //
  // Get the length of the item
  //
  Item              = ps->Item;
  Item->Item.Index  = 0;
  while (Item->Item.Index < Item->FieldWidth) {
    c = PGETC (&Item->Item);
    if (!c) {
      Item->Item.Index -= 1;
      break;
    }
  }

  Len = Item->Item.Index;

  //
  // if there is no item field width, use the items width
  //
  if (Item->FieldWidth == (UINTN) -1) {
    Item->FieldWidth = Len;
  }
  //
  // if item is larger then width, update width
  //
  if (Len > Item->Width) {
    Item->Width = Len;
  }
  //
  // if pad field before, add pad char
  //
  if (Item->PadBefore) {
    for (i = Item->Width; i < Item->FieldWidth; i += 1) {
      PPUTC (ps, ' ');
    }
  }
  //
  // pad item
  //
  for (i = Len; i < Item->Width; i++) {
    PPUTC (ps, Item->Pad);
  }
  //
  // add the item
  //
  Item->Item.Index = 0;
  while (Item->Item.Index < Len) {
    PPUTC (ps, PGETC (&Item->Item));
  }
  //
  // If pad at the end, add pad char
  //
  if (!Item->PadBefore) {
    for (i = Item->Width; i < Item->FieldWidth; i += 1) {
      PPUTC (ps, ' ');
    }
  }
}

UINTN
_Print (
  IN PRINT_STATE     *ps
  )
/*++

Routine Description:

  %w.lF   -   w = width
                l = field width
                F = format of arg

  Args F:
    0       -   pad with zeros
    -       -   justify on left (default is on right)
    ,       -   add comma's to field    
    *       -   width provided on stack
    n       -   Set output attribute to normal (for this field only)
    h       -   Set output attribute to highlight (for this field only)
    e       -   Set output attribute to error (for this field only)
    b       -   Set output attribute to blue color (for this field only)
    v       -   Set output attribute to green color (for this field only)
    l       -   Value is 64 bits

    a       -   ascii string
    s       -   unicode string
    X       -   fixed 8 byte value in hex
    x       -   hex value
    d       -   value as decimal    
    c       -   Unicode char
    t       -   EFI time structure
    g       -   Pointer to GUID
    r       -   EFI status code (result code)

    N       -   Set output attribute to normal
    H       -   Set output attribute to highlight
    E       -   Set output attribute to error
    B       -   Set output attribute to blue color
    V       -   Set output attribute to green color
    %       -   Print a %
    
Arguments:

    ps - Ps

Returns:

  Number of charactors written   

--*/
{
  CHAR16      c;
  UINTN       Attr;
  PRINT_ITEM  Item;
  CHAR16      *Buffer;
  EFI_GUID    *TmpGUID;

  //
  // If Omit print to ConOut, then return 0.
  //

  Item.Scratch = AllocateZeroPool (sizeof (CHAR16) * PRINT_ITEM_BUFFER_LEN);
  if (NULL == Item.Scratch) {
    return EFI_OUT_OF_RESOURCES;
  }

  Buffer = AllocateZeroPool (sizeof (CHAR16) * PRINT_STRING_LEN);
  if (NULL == Buffer) {
    FreePool (Item.Scratch);
    return EFI_OUT_OF_RESOURCES;
  }

  ps->Len       = 0;
  ps->Buffer    = (CHAR16 *) ((CHAR8 *) Buffer + PRINT_JOINT_BUFFER_LEN);
  ps->Pos       = ps->Buffer;
  ps->End       = Buffer + PRINT_STRING_LEN - 1;
  ps->Item      = &Item;

  ps->fmt.Index = 0;
  c             = PGETC (&ps->fmt);
  while (c) {

    if (c != '%') {
      PPUTC (ps, c);
      if (Trace) Data[Index++] = c;
      c = PGETC (&ps->fmt);
      continue;
    }
    //
    // setup for new item
    //
    Item.FieldWidth = (UINTN) -1;
    Item.Width      = 0;
    Item.WidthParse = &Item.Width;
    Item.Pad        = ' ';
    Item.PadBefore  = TRUE;
    Item.Comma      = FALSE;
    Item.Long       = FALSE;
    Item.Item.Ascii = FALSE;
    Item.Item.u.pw  = NULL;
    ps->RestoreAttr = 0;
    Attr            = 0;

    c               = PGETC (&ps->fmt);
    while (c) {

      switch (c) {

      case '%':
        //
        // %% -> %
        //
        Item.Item.u.pw    = Item.Scratch;
        Item.Item.u.pw[0] = '%';
        Item.Item.u.pw[1] = 0;
        break;

      case '0':
        Item.Pad = '0';
        break;

      case '-':
        Item.PadBefore = FALSE;
        break;

      case ',':
        Item.Comma = TRUE;
        break;

      case '.':
        Item.WidthParse = &Item.FieldWidth;
        break;

      case '*':
        *Item.WidthParse = VA_ARG (ps->args, UINTN);
        break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        *Item.WidthParse = 0;
        do {
          *Item.WidthParse  = *Item.WidthParse * 10 + c - '0';
          c                 = PGETC (&ps->fmt);
        } while (c >= '0' && c <= '9');
        ps->fmt.Index -= 1;
        break;

      case 'a':
        Item.Item.u.pc  = VA_ARG (ps->args, CHAR8 *);
        Item.Item.Ascii = TRUE;
        if (!Item.Item.u.pc) {
          Item.Item.u.pc = "(null)";
        }

        Item.PadBefore = FALSE;
        break;

      case 's':
        Item.Item.u.pw = VA_ARG (ps->args, CHAR16 *);
        if (!Item.Item.u.pw) {
          Item.Item.u.pw = L"(null)";
        }

        Item.PadBefore = FALSE;
        break;

      case 'c':
        Item.Item.u.pw    = Item.Scratch;
        Item.Item.u.pw[0] = (CHAR16) VA_ARG (ps->args, UINTN);
        Item.Item.u.pw[1] = 0;
        break;

      case 'l':
        Item.Long = TRUE;
        break;

      case 'X':
        Item.Width  = Item.Long ? 16 : 8;
        Item.Pad    = '0';

      case 'x':
        Item.Item.u.pw = Item.Scratch;
        ValueToHex (
          Item.Item.u.pw,
          Item.Long ? VA_ARG (ps->args, UINT64) : VA_ARG (ps->args, UINTN)
          );

        break;

      case 'd':
        Item.Item.u.pw = Item.Scratch;
        ValueToString (
          Item.Item.u.pw,
          Item.Comma,
          Item.Long ? VA_ARG64 (ps->args, UINT64) : VA_ARG (ps->args, INTN)
          );
        break;


      case 'n':
        PSETATTR (ps, ps->AttrNorm);
        break;

      case 'h':
        PSETATTR (ps, ps->AttrHighlight);
        break;

      case 'b':
        PSETATTR (ps, ps->AttrBlueColor);
        break;

      case 'v':
        PSETATTR (ps, ps->AttrGreenColor);
        break;

      case 'e':
        PSETATTR (ps, ps->AttrError);
        break;

      case 'N':
        Attr = ps->AttrNorm;
        break;

      case 'H':
        Attr = ps->AttrHighlight;
        break;

      case 'E':
        Attr = ps->AttrError;
        break;

      case 'B':
        Attr = ps->AttrBlueColor;
        break;

      case 'V':
        Attr = ps->AttrGreenColor;
        break;

      default:
        Item.Item.u.pw    = Item.Scratch;
        Item.Item.u.pw[0] = '?';
        Item.Item.u.pw[1] = 0;
        break;
      }
      //
      // if we have an Item
      //
      if (Item.Item.u.pw) {
        PITEM (ps);
        break;
      }
      //
      // if we have an Attr set
      //
      if (Attr) {
        PSETATTR (ps, Attr);
        ps->RestoreAttr = 0;
        break;
      }

      c = PGETC (&ps->fmt);
    }

    if (ps->RestoreAttr) {
      PSETATTR (ps, ps->RestoreAttr);
    }

    c = PGETC (&ps->fmt);
  }
  //
  // Flush buffer
  //
  PFLUSH (ps);

  FreePool (Item.Scratch);
  FreePool (Buffer);

  return ps->Len;
}


STATIC CHAR8  Hex[] = {
  '0',
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  'A',
  'B',
  'C',
  'D',
  'E',
  'F'
};


CHAR8  ca[] = { 3, 1, 2 };


VOID
ValueToHex (
  IN CHAR16   *Buffer,
  IN UINT64   v
  )
{
  CHAR8   str[30];

  CHAR8   *p1;
  CHAR16  *p2;

  if (!v) {
    Buffer[0] = '0';
    Buffer[1] = 0;
    return ;
  }

  p1  = str;
  p2  = Buffer;

  while (v) {
    *(p1++) = Hex[v & 0xf];
    v       = RShiftU64 (v, 4);
  }

  while (p1 != str) {
    *(p2++) = *(--p1);
  }

  *p2 = 0;
}


VOID
ValueToString (
  IN CHAR16   *Buffer,
  IN BOOLEAN  Comma,
  IN INT64    v
  )
{
  CHAR8         str[40];
  CHAR8         *p1;
  CHAR16        *p2;
  UINTN         c;
  UINTN         r;


  if (Trace) Data[Index++] = (UINT32)v;
  if (Trace) Data[Index++] = (UINT32)(v>>32);

  if (!v) {
    Buffer[0] = '0';
    Buffer[1] = 0;
    return ;
  }

  p1  = str;
  p2  = Buffer;

  if (v < 0) {
    *(p2++) = '-';
    v       = -v;
  }

  while (v) {
    v       = (INT64) DivU64x32 ((UINT64) v, 10, &r);
    *(p1++) = (CHAR8) ((CHAR8) r + '0');
  }

  c = (Comma ? ca[(p1 - str) % 3] : 999) + 1;
  while (p1 != str) {

    c -= 1;
    if (!c) {
      *(p2++) = ',';
      c       = 3;
    }

    *(p2++) = *(--p1);
  }

  *p2 = 0;
}












EFI_STATUS
EFIAPI
main (
  )
/*++

Routine Description:

Arguments:

  ImageHandle     - The handle for this driver

  SystemTable     - The system table

Returns:

--*/
{


  UINTN  BufSize;
  UINT64 FileSize;
  UINT32 i;


  printfOut.OutputString = printfOutputString;

      BufSize = 1;
      FileSize = 4681;

      Trace = 1;
      Print(L"BS %x FS %11,ld\n", BufSize, FileSize);
      Print(L"FS %11,ld\n", FileSize);
      Print(L"BS %x FS %11,ld\n", BufSize, FileSize);
      Trace = 0;

      for (i = 0; i < Index; i++) {
        Print(L"0x%x\n", Data[i]);
      }
      
      //while (1);

  return EFI_SUCCESS;
}

