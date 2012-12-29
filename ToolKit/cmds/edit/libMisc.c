/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libMisc.c

  Abstract:
    Implementation of various string and line routines

--*/


#ifndef _LIB_MISC
#define _LIB_MISC

#include "editor.h"



//
// Name:
//		EditorClearLine -- Clear line at Row
// In:
// 		Row -- row number to be cleared ( start from 1 );
// Out:
// 		EFI_SUCCESS
//
 

VOID EditorClearLine ( UINTN Row )
{
	CHAR16 Line[200];
	UINTN i;
	
	// prepar a blank line
	for ( i = 0; i < DISPLAY_MAX_COLUMN; i++ ) {
		Line[i] = ' ';
	}
	
	if ( Row == DISPLAY_MAX_ROW ) {
		// if '\0' is still at postion 80, it will cause first line error
		Line[DISPLAY_MAX_COLUMN - 1 ] = '\0';
	} else {
		Line[DISPLAY_MAX_COLUMN ] = '\0';
	}
	
	// print out the blank line
	PrintAt ( 0, Row -1 , Line );
}





//
// Name:
//		LineDup -- Duplicate a line
// In:
// 		Src -- line to be duplicated
// Out:
// 		NULL -- wrong
//		Not NULL -- line created
//

EFI_EDITOR_LINE*
LineDup ( 
    IN  EFI_EDITOR_LINE *Src
    )
{
    EFI_EDITOR_LINE *Dest;
    Dest = AllocatePool(sizeof(EFI_EDITOR_LINE));
    if ( Dest == NULL ) {
    	return NULL;
    }
    
    Dest->Signature = EFI_EDITOR_LINE_LIST;
    Dest->Size = Src->Size;
    Dest->Buffer = PoolPrint(L"%s\0",Src->Buffer);
    if ( Dest -> Buffer == NULL ) {
    	FreePool ( Dest );
    	return NULL;
    }

    Dest->Link = Src->Link;

    return Dest;
}






//
// Name:
//		LineFree -- Free a line and it's internal buffer
// In:
// 		Src -- line to be freed
// Out:
// 		EFI_SUCCESS
//

VOID
LineFree ( 
    IN  EFI_EDITOR_LINE *Src
    )
{
	if ( Src == NULL ) {
		return;
	}
	
	EditorFreePool ( Src -> Buffer );
	EditorFreePool ( Src );
	
}




//
// Name:
//		EditorFreePool -- Free a pointed area.
//			its different with FreePool is that it can free NULL pointer
// In:
// 		Buffer -- Buffer to be freed
// Out:
// 		VOID
//

VOID
EditorFreePool (
    IN VOID            *Buffer
    )
{
	if ( Buffer == NULL ) {
		return ;
	}
	
	FreePool ( Buffer );
}






//
// Name:
//		_LineAdvance -- Advance to the next Count lines
// In:
// 		Count -- line number to advance
// Out:
// 		NULL -- wrong
//		Not NULL -- line after advance
//

EFI_EDITOR_LINE*
_LineAdvance (
    IN  UINTN Count
    )
{
    UINTN   i;
	EFI_EDITOR_LINE * Line;

	Line = MainEditor.FileBuffer->CurrentLine;
	if (  Line == NULL ) {
		return NULL;
	}

	for ( i = 0 ; i < Count ; i++ ) {
		// if already last line
		if ( Line->Link.Flink == MainEditor.FileBuffer->ListHead ) {
			return NULL;
		}

		Line = CR(Line->Link.Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
	}

    return Line;
}






//
// Name:
//		_LineRetreat -- Retreat to the previous Count lines
// In:
// 		Count -- line number to retreat
// Out:
// 		NULL -- wrong
//		Not NULL -- line after retreat
//

EFI_EDITOR_LINE*
_LineRetreat (
    IN  UINTN Count
    )
{ 
    UINTN   i;
	EFI_EDITOR_LINE * Line;

	Line = MainEditor.FileBuffer->CurrentLine;
	if (  Line == NULL ) {
		return NULL;
	}

	for ( i = 0 ; i < Count ; i++ ) {
		
		// already the first line
		if ( Line->Link.Blink == MainEditor.FileBuffer->ListHead ) {
			return NULL;
		}

		Line = CR(Line->Link.Blink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
	}

    return Line;
}






//
// Name:
//		MoveLine -- Advance/Retreat lines
// In:
// 		Count -- line number to advance/retreat
//			>0 : advance
//			<0: retreat
// Out:
// 		NULL -- wrong
//		Not NULL -- line after advance
//

EFI_EDITOR_LINE*
MoveLine (
    IN  INTN Count
    )
{ 
	EFI_EDITOR_LINE *Line;
	UINTN AbsCount;
	
	if ( Count <=0 ) {
		AbsCount = -Count; 
		Line = _LineRetreat ( AbsCount );
	} else {
		Line = _LineAdvance ( Count );
	}
	
	return Line;
}






//
// Name:
//		MoveLine -- Advance/Retreat lines and set CurrentLine in FileBuffer to it
// In:
// 		Count -- line number to advance/retreat
//			>0 : advance
//			<0: retreat
// Out:
// 		NULL -- wrong
//		Not NULL -- line after advance
//

EFI_EDITOR_LINE*
MoveCurrentLine (
    IN  INTN Count
    )
{ 
	EFI_EDITOR_LINE *Line;
	UINTN AbsCount;
	
	if ( Count <=0 ) {
		AbsCount = -Count; 
		Line = _LineRetreat ( AbsCount );
	} else {
		Line = _LineAdvance ( Count );
	}
	
	if ( Line == NULL ) {
		return NULL ;
	};
	
	MainEditor.FileBuffer->CurrentLine = Line;
	
	return Line;
}






//
// Name:
//		LineStrInsert -- Insert a char into line
// In:
// 		Line -- line to insert into
//		Char -- char to insert
//		Pos -- Position to insert the char at ( start from 0 )
//		StrSize -- current string size ( include '\0' ) ( unit is unicode character )
// Out:
//		new string size ( include '\0' ) ( unit is unicode character )
//

UINTN
LineStrInsert (
    IN		EFI_EDITOR_LINE  *Line,
    IN      CHAR16  Char,
    IN      UINTN   Pos,
    IN      UINTN   StrSize
    )
{
    UINTN   i;
    CHAR16  *s;
	CHAR16 *Str;

    i = (StrSize)*2;

	Str = Line -> Buffer;
	
	// do not have free space
	if ( Line -> TotalSize <= Line -> Size ) {
		Str = ReallocatePool( Str, i , i + 16);
		if ( Str == NULL )
			return 0;
			
		Line -> TotalSize += 8;
	}

    // move the later part of the string one character right 
    s = Str;
    for (i = StrSize; i > Pos; i--) {
        s[i] = s[i-1];
    }
    
    // insert char intot it.
    s[i] = Char;

	Line -> Buffer = Str;
	Line -> Size++;


    return (StrSize+1);
}






//
// Name:
//		LineDeleteAt -- Delete a char in line
// In:
// 		Line -- line to delete in
//		Pos -- Position to delete the char at ( start from 0 )
// Out:
//		VOID
//

VOID
LineDeleteAt (
    IN  OUT EFI_EDITOR_LINE*    Line,
    IN      UINTN               Pos
    )
{
    UINTN   i;


    for ( i = Pos - 1; i < Line->Size; i++) {
        Line->Buffer[i] = Line->Buffer[i+1];
    }

    --Line->Size;
}






//
// Name:
//		LineCat -- Cat Src into Dest
// In:
// 		Dest -- Destination string
//		Src -- Src String
// Out:
//		VOID
//

VOID
LineCat (
    IN  OUT EFI_EDITOR_LINE* Dest,
    IN      EFI_EDITOR_LINE* Src
    )
{
    CHAR16  *Str;
    UINTN   Size;

    Size = Dest->Size ;

    Dest->Buffer[Size] = 0;

    Str = PoolPrint (L"%s%s",Dest->Buffer,Src->Buffer);
	if ( Str == NULL ) {
		Dest -> Buffer = NULL;
		return;
	}

    Dest->Size = Size + Src->Size;
    Dest->TotalSize = Dest->Size;

    FreePool (Dest->Buffer);
    FreePool (Src->Buffer);

    Dest->Buffer = Str;
}







//
// Name:
//		UnicodeToAscii -- change a unicode string t ascii string
// In:
// 		UStr -- unicode string 
//		Lenght -- most possible length of AStr
//		AStr -- Ascii string to pas sout
// Out:
//		Actuall length
//

UINTN
UnicodeToAscii  (
    IN  CHAR16  *UStr,
    IN  UINTN   Length,
    OUT CHAR8   *AStr
    )
{
    UINTN   i;

    for (i = 0; i < Length; i++) {
        *AStr++ = (CHAR8)*UStr++;
    }

    return i;
}







//
// Name:
//		StrStr -- Search Pat in Str
// In:
// 		Str -- mother string
//		Pat -- search pattern
// Out:
//		 0 : not found
// 		>= 1 : found position + 1
//

UINTN
StrStr  (
    IN  CHAR16  *Str,
    IN  CHAR16  *Pat
    )
{
    INTN    *Failure;
    INTN    i,j;
    INTN    Lenp;
    INTN    Lens;

    Lenp = StrLen(Pat);
    Lens = StrLen(Str);

    Failure = AllocatePool(Lenp*sizeof(INTN));
    Failure[0] = -1;
    for (j=1; j< Lenp; j++ ) {
        i = Failure[j-1];
        while ( (Pat[j] != Pat[i+1]) && (i >= 0)) {
            i = Failure[i];
        }
        if ( Pat[i] == Pat[i+1]) {
            Failure[j] = i+1;
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
            j = Failure[j-1] + 1;
        }
    }

    FreePool(Failure);

    return ((j == Lenp) ? (i - Lenp) : -1)+1;

}




static CHAR8 OtherChars[] = {'0','1','2','3','4','5','6','7',
                             '8','9','\\','.','_','^','$','~',
                             '!','#','%','&','-','{','}','(',
                             ')','@','`','\'',':'};
BOOLEAN IsValidFileNameChar ( CHAR16 Ch )
{
UINTN OtherCharsLen = 29;
UINTN i;

	if ( Ch >= L'a' && Ch <= L'z' ) 
		return TRUE;

	if ( Ch >= L'A' && Ch <= L'Z' ) 
		return TRUE;

	if ( Ch >= 0xe0 && Ch <= 0xf6 ) 
		return TRUE;

	if ( Ch >= (0xe0 - 0x20) && Ch <= (0xf6 - 0x20) ) 
		return TRUE;

	if ( Ch >= 0xf8 && Ch <= 0xfe ) 
		return TRUE;

	if ( Ch >= (0xf8 - 0x20) && Ch <= (0xfe - 0x20) ) 
		return TRUE;


	
	for ( i = 0 ; i < OtherCharsLen; i++ ) {
		if ( Ch == OtherChars[i] )
			return TRUE;
	}

	return FALSE;
}


//
// Name:
//		IsValidFileName -- check if file name has illegal character
// In:
// 		Name -- file name
// Out:
//		 TRUE : valid
// 		FALSE : invalid 
//

BOOLEAN 
IsValidFileName ( CHAR16 *Name ) {

UINTN i;

	for ( i = 0 ; i < StrLen ( Name ); i++ )
		if ( !IsValidFileNameChar ( Name[i] ) )
			return FALSE;
	
	return TRUE;
}



#endif 	
