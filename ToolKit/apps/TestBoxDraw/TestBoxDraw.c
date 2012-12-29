/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    TestBoxDraw.c
    
Abstract:
    Test of Unicode Box Draw characters. This test draw the four box types supported 
    by the Unicode Box Draw characters. It also draws a acsii based box. The ascii based
    box can be used for devices that do not support the Box Characters.


Revision History

--*/

#include "efi.h"
#include "efilib.h"

CHAR16  TopLine[] = { 
    BOXDRAW_DOWN_RIGHT, BOXDRAW_HORIZONTAL, BOXDRAW_DOWN_HORIZONTAL, BOXDRAW_HORIZONTAL, BOXDRAW_DOWN_LEFT, L' ', L' ',
    BOXDRAW_DOWN_RIGHT_DOUBLE, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOWN_HORIZONTAL_DOUBLE, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOWN_LEFT_DOUBLE, L' ', L' ',
    BOXDRAW_DOUBLE_DOWN_RIGHT, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOUBLE_DOWN_HORIZONTAL, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOUBLE_DOWN_LEFT, L' ', L' ',
    BOXDRAW_DOWN_DOUBLE_RIGHT, BOXDRAW_HORIZONTAL, BOXDRAW_DOWN_DOUBLE_HORIZONTAL, BOXDRAW_HORIZONTAL, BOXDRAW_DOWN_DOUBLE_LEFT, L' ', L' ',
    L'/', L'-', L'+', L'-', L'\\', L' ', L' ',
    0x0000 
};

CHAR16  MiddleGap[] = {
    BOXDRAW_VERTICAL, L' ', BOXDRAW_VERTICAL, L' ', BOXDRAW_VERTICAL, L' ', L' ', 
    BOXDRAW_VERTICAL, L' ', BOXDRAW_VERTICAL, L' ', BOXDRAW_VERTICAL, L' ', L' ', 
    BOXDRAW_DOUBLE_VERTICAL, L' ', BOXDRAW_DOUBLE_VERTICAL, L' ', BOXDRAW_DOUBLE_VERTICAL, L' ', L' ', 
    BOXDRAW_DOUBLE_VERTICAL, L' ', BOXDRAW_DOUBLE_VERTICAL, L' ', BOXDRAW_DOUBLE_VERTICAL, L' ', L' ', 
    L'|', L' ', L'|', L' ', L'|', L' ', L' ',
    0x0000
};

CHAR16  MiddleLine[] = { 
    BOXDRAW_VERTICAL_RIGHT, BOXDRAW_HORIZONTAL, BOXDRAW_VERTICAL_HORIZONTAL, BOXDRAW_HORIZONTAL, BOXDRAW_VERTICAL_LEFT, L' ', L' ', 
    BOXDRAW_VERTICAL_RIGHT_DOUBLE, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_VERTICAL_LEFT_DOUBLE, L' ', L' ', 
    BOXDRAW_DOUBLE_VERTICAL_RIGHT, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOUBLE_VERTICAL_LEFT, L' ', L' ', 
    BOXDRAW_VERTICAL_DOUBLE_RIGHT, BOXDRAW_HORIZONTAL, BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL, BOXDRAW_HORIZONTAL, BOXDRAW_VERTICAL_DOUBLE_LEFT, L' ', L' ', 
    L'|', L'-', L'+', L'-', L'|', L' ', L' ',
    0x0000
};

CHAR16  BottomLine[] = { 
    BOXDRAW_UP_RIGHT, BOXDRAW_HORIZONTAL, BOXDRAW_UP_HORIZONTAL, BOXDRAW_HORIZONTAL, BOXDRAW_UP_LEFT, L' ', L' ',
    BOXDRAW_UP_RIGHT_DOUBLE, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_UP_HORIZONTAL_DOUBLE, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_UP_LEFT_DOUBLE, L' ', L' ',
    BOXDRAW_DOUBLE_UP_RIGHT, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOUBLE_UP_HORIZONTAL, BOXDRAW_DOUBLE_HORIZONTAL, BOXDRAW_DOUBLE_UP_LEFT, L' ', L' ',
    BOXDRAW_UP_DOUBLE_RIGHT, BOXDRAW_HORIZONTAL, BOXDRAW_UP_DOUBLE_HORIZONTAL, BOXDRAW_HORIZONTAL, BOXDRAW_UP_DOUBLE_LEFT, L' ', L' ',
    L'\\', L'-', L'+', L'-', L'/', L' ', L' ',
    0x0000
};

UINTN EfiColor[] = {
    EFI_BLACK,  
    EFI_BLUE,    
    EFI_GREEN,   
    EFI_CYAN,    
    EFI_RED,     
    EFI_MAGENTA, 
    EFI_BROWN,   
    EFI_LIGHTGRAY,                 
    EFI_DARKGRAY,        
    EFI_LIGHTBLUE,       
    EFI_LIGHTGREEN,      
    EFI_LIGHTCYAN,       
    EFI_LIGHTRED,        
    EFI_LIGHTMAGENTA,    
    EFI_YELLOW,          
    EFI_WHITE           
};

UINTN   EfiBackGroundColor[] = {
    EFI_BACKGROUND_BLACK,        
    EFI_BACKGROUND_BLUE,         
    EFI_BACKGROUND_GREEN,        
    EFI_BACKGROUND_CYAN,         
    EFI_BACKGROUND_RED,          
    EFI_BACKGROUND_MAGENTA,      
    EFI_BACKGROUND_BROWN,        
    EFI_BACKGROUND_LIGHTGRAY
};



EFI_STATUS
InitializeTestDrawBox (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    INTN i;

    InitializeLib (ImageHandle, SystemTable);
    Print (L"%A", EFI_WHITE | EFI_BACKGROUND_BLACK);
    ST->ConOut->ClearScreen (ST->ConOut);

    Print (L"Box Draw Test Application Started\n");

    Print (L"%s\n", TopLine);
    Print (L"%s\n", MiddleGap);
    Print (L"%s\n", MiddleLine);
    Print (L"%s\n", MiddleGap);
    Print (L"%s\n", BottomLine);

    //
    // Print out Triangles followed by Ascii equivelents.
    //
    Print (L"\n%c ^  %c v  %c >  %c <\n\n", 
            GEOMETRICSHAPE_UP_TRIANGLE, GEOMETRICSHAPE_DOWN_TRIANGLE,
            GEOMETRICSHAPE_RIGHT_TRIANGLE, GEOMETRICSHAPE_LEFT_TRIANGLE
            );

    Print (L"\n%c ^  %c v\n\n", 
            ARROW_UP, ARROW_DOWN
            );

    //
    // Print Progress Bar
    //
    for (i=0; i<20; i++) {
        if (i <= 10) {
            Print (L"%ec", BLOCKELEMENT_FULL_BLOCK);
        } else {
            Print (L"%c", BLOCKELEMENT_LIGHT_SHADE);
        }
    }
    Print (L"\n\n");

    //
    // Print Progress Bar ASCII
    //
    for (i=0; i<20; i++) {
        if (i <= 10) {
            Print (L"%ec", L'#');
        } else {
            Print (L"%c", L'+');
        }
    }
    Print (L"\n");

    //
    // Test colors
    //
    for (i=0; i<=7; i++) {
        ST->ConOut->SetAttribute (ST->ConOut, EFI_WHITE | EfiBackGroundColor[i]);
        Print (L"X");
    }
    Print (L"\n");
    
    for (i=0; i<=0x0f; i++) {
        ST->ConOut->SetAttribute (ST->ConOut, EfiColor[i] | EFI_BACKGROUND_BLACK);
        Print (L"%c", BLOCKELEMENT_FULL_BLOCK);
    }
    Print (L"\n");

    for (i=0; i<=0x0f; i++) {
        ST->ConOut->SetAttribute (ST->ConOut, EfiColor[i] | EFI_BACKGROUND_BLACK);
        Print (L"%c", BLOCKELEMENT_LIGHT_SHADE);
    }
    Print (L"\n");

    //
    // Print VERTICAL Bars
    //
    PrintAt (69, 4, L"%c%c%c /-\\", 
                BOXDRAW_DOWN_RIGHT_DOUBLE,
                BOXDRAW_DOUBLE_HORIZONTAL,
                BOXDRAW_DOWN_LEFT_DOUBLE
                );
    PrintAt (69, 20, L"%c%c%c \\-/", 
                BOXDRAW_UP_RIGHT_DOUBLE,
                BOXDRAW_DOUBLE_HORIZONTAL,
                BOXDRAW_UP_LEFT_DOUBLE
                );
    for (i=5; i<20; i++) {
        PrintAt (69, i, L"%c %c | |", BOXDRAW_VERTICAL, BOXDRAW_VERTICAL);
        if (i == 10) {
            PrintAt (70, i, L"%ec", BLOCKELEMENT_FULL_BLOCK);
            PrintAt (74, i, L"%ec", L'#');
        } else {
            PrintAt (70, i, L"%c", BLOCKELEMENT_LIGHT_SHADE);
            PrintAt (74, i, L"%c", L'+');
        }
    }

    PrintAt (0, 23, L"\n");
    return EFI_SUCCESS;
}

