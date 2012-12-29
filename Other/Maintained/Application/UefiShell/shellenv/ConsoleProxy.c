/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 
  consoleproxy.c

Abstract:

  Implementation of the Console Proxy. The ability to lay the customized ConIn proxy, 
  ConOut proxy on the standard ConIn, ConOut, so as to extend the standard console 
  to provide the addtional feature and control. Currently, ConIn proxy can support
  timely key detection. And ConOut proxy can support scroll back buffer.
 
--*/

#include "shelle.h"

typedef struct {
  UINT32        Count;
  UINT32        StartPos;
  UINT32        EndPos;
  EFI_INPUT_KEY *Buffer;
} EFI_KEY_BUFFER;

#define INPUT_STATE_DEFAULT                 0x00
#define INPUT_STATE_ESC                     0x01
#define OUTPUT_PAUSE_KEY(Key)               ((Key)->UnicodeChar == 0x09 && (Key)->ScanCode == 0)
#define OUTPUT_BREAK_KEY(Key)               ((Key)->UnicodeChar == 0 && (Key)->ScanCode == SCAN_ESC)

#define CON_IN_PROXY_PRIVATE_DATA_SIGNATURE EFI_SIGNATURE_32 ('c', 'i', 'p', 's')

typedef struct {
  UINTN                       Signature;
  UINT32                      KeyFilter;
  EFI_KEY_BUFFER              KeyBuffer;
  EFI_EVENT                   ConInEvent;
  EFI_EVENT                   TwoSecondTimeOut;
  UINT32                      InputState;
  UINTN                       OpenedCount;
  //
  // Saved original console in protocol
  //
  EFI_SIMPLE_TEXT_IN_PROTOCOL *SavedConIn;
  //
  // Proxy console in protocol
  //
  EFI_SIMPLE_TEXT_IN_PROTOCOL ProxyConIn;
} CON_IN_PROXY_PRIVATE_DATA;

#define CON_IN_PROXY_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      CON_IN_PROXY_PRIVATE_DATA, \
      ProxyConIn, \
      CON_IN_PROXY_PRIVATE_DATA_SIGNATURE \
      )

typedef struct {
  BOOLEAN PageBreak;
  BOOLEAN AutoWrap;
  BOOLEAN OmitPrint;
  BOOLEAN OutputPause;
  UINTN   MaxRow;
  UINTN   MaxColumn;
  UINTN   InitRow;
  UINTN   Row;
  UINTN   Column;
} PRINT_MODE;

#define CON_OUT_PROXY_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('c', 'o', 'p', 's')

typedef struct {
  UINTN                         Signature;
  //
  // Print mode control
  //
  PRINT_MODE                    PrintMode;
  //
  // Variables to maintain the console output history
  //
  BOOLEAN                       HistoryValid;
  BOOLEAN                       HistoryEnabled;
  UINTN                         HistoryColumnsPerScreen;
  UINTN                         HistoryRowsPerScreen;
  UINTN                         HistoryScreens;
  UINTN                         HistoryColumns;
  UINTN                         HistoryRows;
  UINTN                         HistoryVisibleStartRow;
  UINTN                         HistoryOriginalStartRow;
  CHAR16                        *HistoryScreen;
  INT32                         *HistoryAttributes;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   HistoryMode;
  UINTN                         OpenedCount;
  //
  // Saved original console out protocol
  //
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SavedConOut;
  //
  // Proxy console out protocol
  //
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  ProxyConOut;
} CON_OUT_PROXY_PRIVATE_DATA;

#define CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      CON_OUT_PROXY_PRIVATE_DATA, \
      ProxyConOut, \
      CON_OUT_PROXY_PRIVATE_DATA_SIGNATURE \
      )

#define CONSOLE_OUTPUT_HISTORY_SCREENS  3
#define KEY_BUFFER_MAX_COUNT            32

//
// Proxy console input protocol interfaces
//
EFI_STATUS
EFIAPI
ProxyConInReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  );

EFI_STATUS
EFIAPI
ProxyConInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *This,
  OUT EFI_INPUT_KEY                   *Key
  );

VOID
EFIAPI
ProxyConInWaitForKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

VOID
ProxyConInResetKeyBuf (
  VOID
  );

EFI_STATUS
ProxyConInReadKeyBuf (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  OUT EFI_INPUT_KEY                  *Key
  );

VOID
ProxyConInAddKeyBuf (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  IN  EFI_INPUT_KEY                  Key
  );

VOID
EFIAPI
ProxyConInCheckKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

EFI_STATUS
ProxyConInKeyFilter (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  IN  EFI_INPUT_KEY                  *Key
  );

EFI_STATUS
ProxyConInFlashState (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  IN  EFI_INPUT_KEY                  *Key
  );

//
// Proxy console output protocol interfaces
//
EFI_STATUS
EFIAPI
ProxyConOutReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  );

EFI_STATUS
EFIAPI
ProxyConOutOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
EFIAPI
ProxyConOutTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
EFIAPI
ProxyConOutQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber,
  OUT UINTN                       *Columns,
  OUT UINTN                       *Rows
  );

EFI_STATUS
EFIAPI
ProxyConOutSetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber
  );

EFI_STATUS
EFIAPI
ProxyConOutSetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN UINTN                            Attribute
  );

EFI_STATUS
EFIAPI
ProxyConOutClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This
  );

EFI_STATUS
EFIAPI
ProxyConOutSetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        Column,
  IN UINTN                        Row
  );

EFI_STATUS
EFIAPI
ProxyConOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       Visible
  );

//
// Global Variables
//
STATIC CON_IN_PROXY_PRIVATE_DATA  mConInProxy = {
  CON_IN_PROXY_PRIVATE_DATA_SIGNATURE,
  EFI_OUTPUT_PAUSE | EFI_EXECUTION_BREAK,
  {
    0,
    0,
    0,
    NULL,
  },

  (EFI_EVENT) NULL,
  (EFI_EVENT) NULL,
  0,
  0,

  (EFI_SIMPLE_TEXT_IN_PROTOCOL *) NULL,

  {
    ProxyConInReset,
    ProxyConInReadKeyStroke,
    (EFI_EVENT) NULL,
  }
};

STATIC CON_OUT_PROXY_PRIVATE_DATA mConOutProxy = {
  CON_OUT_PROXY_PRIVATE_DATA_SIGNATURE,
  {
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    0,
    0,
    0,
    0,
    0
  },
  FALSE,
  FALSE,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  (CHAR16 *) NULL,
  (INT32 *) NULL,
  {
    1,
    0,
    0,
    0,
    0,
    FALSE
  },
  0,
  (EFI_SIMPLE_TEXT_OUT_PROTOCOL *) NULL,
  {
    ProxyConOutReset,
    ProxyConOutOutputString,
    ProxyConOutTestString,
    ProxyConOutQueryMode,
    ProxyConOutSetMode,
    ProxyConOutSetAttribute,
    ProxyConOutClearScreen,
    ProxyConOutSetCursorPosition,
    ProxyConOutEnableCursor,
    (EFI_SIMPLE_TEXT_OUTPUT_MODE *) NULL
  }
};

STATIC CHAR16                     mCrLfString[3] = { CHAR_CARRIAGE_RETURN, CHAR_LINEFEED, CHAR_NULL };

//
// Private worker function prototypes
//
STATIC
VOID
ConInProxyOpen (
  IN     EFI_HANDLE                   ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL  **ConIn
  );

STATIC
VOID
ConInProxyClose (
  IN     EFI_HANDLE                ConInHandle,
  OUT EFI_SIMPLE_TEXT_IN_PROTOCOL  **ConIn
  );

STATIC
VOID
ConOutProxyOpen (
  IN     EFI_HANDLE                    ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL  **ConOut
  );

STATIC
VOID
ConOutProxyClose (
  IN     EFI_HANDLE                    ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL  **ConOut
  );

STATIC
EFI_STATUS
ConOutHistoryCreate (
  VOID
  );

STATIC
VOID
ConOutHistoryDestory (
  VOID
  );

STATIC
EFI_STATUS
ConOutHistoryUpdateVisibleScreen (
  VOID
  );

STATIC
EFI_STATUS
ConOutHistoryOutputString (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  CHAR16                          *WString
  );

STATIC
EFI_STATUS
ConOutHistorySetMode (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private,
  IN  UINTN                         ModeNumber
  );

STATIC
EFI_STATUS
ConOutHistorySetAttribute (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  UINTN                           Attribute
  );

STATIC
EFI_STATUS
ConOutHistoryClearScreen (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private
  );

STATIC
EFI_STATUS
ConOutHistorySetCursorPosition (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  );

STATIC
EFI_STATUS
ConOutHistoryEnableCursor (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private,
  IN  BOOLEAN                       Visible
  );

STATIC
EFI_STATUS
ConOutOutputString (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  CHAR16                          *WString
  );

STATIC
EFI_STATUS
ConOutOutputStringWithPageBreak (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  CHAR16                          *WString
  );

STATIC
BOOLEAN
ConOutSetPageBreak (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private
  );

STATIC
VOID
ConOutSetOutputPause (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  BOOLEAN                         Pause
  );

STATIC
BOOLEAN
ConOutGetOutputPause (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private
  );

//
// External functions
//
VOID
SEnvOpenConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  )
/*++

Routine Description:
  Open the console proxy to substitute for the passed-in console.
  
Arguments:
  ConInHandle  - The handle of ConIn.

  ConIn        - On input, a pointer to the original console input on which the 
                 proxy console input will be opened.
                 On output, a pointer to the proxy console input.

  ConOutHandle - The handle of ConOut.
  
  ConOut       - On input, a pointer to the original console output on which the 
                 proxy console output will be opened.
                 On output, a pointer to the proxy console output.
               
Returns:

--*/
{
  EFI_TPL OldTpl;

  OldTpl = BS->RaiseTPL (EFI_TPL_NOTIFY);

  ASSERT (ConIn != NULL);
  ASSERT (*ConIn != NULL);
  ASSERT (ConOut != NULL);
  ASSERT (*ConOut != NULL);
  //
  // Open the console input proxy
  //
  ConInProxyOpen (ConInHandle, ConIn);

  //
  // Open the console output proxy
  //
  ConOutProxyOpen (ConOutHandle, ConOut);

  //
  //  Updated CRC for System Table
  //
  SetCrc (&ST->Hdr);

  BS->RestoreTPL (OldTpl);
}

VOID
EFIAPI
SEnvCloseConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  )
/*++

Routine Description:
  Close the console proxy to restore the original console.
  
Arguments:

  ConInHandle  - The handle of ConIn.

  ConIn        - A pointer to the location to return the pointer to the original 
                 console input.

  ConOutHandle - The handle of ConOut.
  
  ConOut       - A pointer to the location to return the pointer to the original 
                 console output.
               
Returns:

--*/
{
  EFI_TPL OldTpl;

  ASSERT (ConIn != NULL);
  ASSERT (ConOut != NULL);

  OldTpl = BS->RaiseTPL (EFI_TPL_NOTIFY);

  //
  // Close the console input proxy
  //
  ConInProxyClose (ConInHandle, ConIn);

  //
  // Close the console output Proxy
  //
  ConOutProxyClose (ConOutHandle, ConOut);

  //
  //  Updated CRC for System Table
  //
  SetCrc (&ST->Hdr);

  BS->RestoreTPL (OldTpl);
}

EFI_STATUS
SEnvCheckConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  )
/*++

Routine Description:
  Check the curretn console to make sure it is proxy console.
  
Arguments:

  ConInHandle  - The handle of ConIn.
  
  ConIn        - On input, a pointer to the original console input on which the 
                 proxy console input will be opened.
                 On output, a pointer to the proxy console input.

  ConOutHandle - The handle of ConOut.

  ConOut       - On input, a pointer to the original console output on which the 
                 proxy console output will be opened.
                 On output, a pointer to the proxy console output.
               
Returns:

  EFI_SUCCESS
             - Current ConIn and ConOut console are proxy console  
  EFI_ABORTED
             - Current ConIn and ConOut console are not proxy console

--*/
{
  if ((*ConIn == &mConInProxy.ProxyConIn) && (*ConOut == &mConOutProxy.ProxyConOut)) {
    return EFI_SUCCESS;
  }

  return EFI_ABORTED;
}

EFI_STATUS
SEnvConOutHistoryEnable (
  IN BOOLEAN      Enable
  )
/*++

  Routine Description:
    Enable/Disable the history screen buffering.

  Arguments:
    Enable  - If TRUE, the history screen buffering is enabled. If FALSE, the 
              history screen buffering is disabled.

  Returns:
    EFI_SUCCESS             - The operation completed successfully.
    EFI_NOT_READY           - The history buffer has not been valid yet.

--*/
{
  if (!mConOutProxy.HistoryValid) {
    return EFI_NOT_READY;
  }

  mConOutProxy.HistoryEnabled = Enable;

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvConOutHistoryScrollBack (
  IN UINTN      *ScrollRows OPTIONAL
  )
/*++

  Routine Description:
    Scroll back the history screen.

  Arguments:
    ScrollRows    - The rows to be scrolled back.

  Returns:
    EFI_SUCCESS           - The operation completed successfully.
    EFI_NOT_READY         - The history buffer has not been valid yet.
    EFI_DEVICE_ERROR      - The device had an error and could not complete the request.
    EFI_UNSUPPORTED       - The output device is not in a valid text mode.

--*/
{
  UINTN Displacement;

  if (!mConOutProxy.HistoryValid) {
    return EFI_NOT_READY;
  }
  //
  // Scroll the history screen back a half screen each time, if ScrollRows is not
  // specified. However, there may be not enough rows left, so we should calculate
  // the actual displacement that can be made.
  //
  if (ScrollRows != NULL) {
    Displacement = *ScrollRows;
  } else {
    Displacement = mConOutProxy.HistoryRowsPerScreen / 2;
  }

  if (mConOutProxy.HistoryVisibleStartRow < Displacement) {
    Displacement = mConOutProxy.HistoryVisibleStartRow;
  }
  //
  // Update the start position of the visible screen.
  //
  if (Displacement == 0) {
    //
    // Need not move. So just return.
    //
    return EFI_SUCCESS;
  } else {
    mConOutProxy.HistoryVisibleStartRow -= Displacement;
  }
  //
  // Update the original console output device with the updated visible screen.
  //
  return ConOutHistoryUpdateVisibleScreen ();
}

EFI_STATUS
SEnvConOutHistoryScrollForward (
  IN UINTN      *ScrollRows OPTIONAL
  )
/*++

  Routine Description:
    Scroll forward the history screen.

  Arguments:
    ScrollRows    - The rows to be scrolled forward.

  Returns:
    EFI_SUCCESS           - The operation completed successfully.
    EFI_NOT_READY         - The history buffer has not been valid yet.
    EFI_DEVICE_ERROR      - The device had an error and could not complete the request.
    EFI_UNSUPPORTED       - The output device is not in a valid text mode.

--*/
{
  UINTN Displacement;

  if (!mConOutProxy.HistoryValid) {
    return EFI_NOT_READY;
  }
  //
  // Scroll the history screen forward a half screen each time, if ScrollRows is not
  // specified. However, there may be not enough rows left, so we should calculate
  // the actual displacement that can be made.
  //
  if (ScrollRows != NULL) {
    Displacement = *ScrollRows;
  } else {
    Displacement = mConOutProxy.HistoryRowsPerScreen / 2;
  }

  if ((mConOutProxy.HistoryOriginalStartRow - mConOutProxy.HistoryVisibleStartRow) < Displacement) {
    Displacement = mConOutProxy.HistoryOriginalStartRow - mConOutProxy.HistoryVisibleStartRow;
  }
  //
  // Update the start position of the visible screen.
  //
  if (Displacement == 0) {
    //
    // Need not move. So just return.
    //
    return EFI_SUCCESS;
  } else {
    mConOutProxy.HistoryVisibleStartRow += Displacement;
  }
  //
  // Update the original console output device with the updated visible screen.
  //
  return ConOutHistoryUpdateVisibleScreen ();
}

EFI_STATUS
SEnvConOutHistoryQuitScroll (
  IN VOID
  )
/*++

  Routine Description:
    Quit scrolling the history screen and restore the original screen.

  Arguments:

    None

  Returns:
    EFI_SUCCESS           - The operation completed successfully.
    EFI_NOT_READY         - The history buffer has not been valid yet.
    EFI_DEVICE_ERROR      - The device had an error and could not complete the request.
    EFI_UNSUPPORTED       - The output device is not in a valid text mode.

--*/
{
  if (!mConOutProxy.HistoryValid) {
    return EFI_NOT_READY;
  }
  //
  // Restore the display of the original screen.
  //
  if (mConOutProxy.HistoryVisibleStartRow == mConOutProxy.HistoryOriginalStartRow) {
    //
    // Need not move. So just return.
    //
    return EFI_SUCCESS;
  } else {
    mConOutProxy.HistoryVisibleStartRow = mConOutProxy.HistoryOriginalStartRow;
  }
  //
  // Update the original console output device with the updated visible screen.
  //
  return ConOutHistoryUpdateVisibleScreen ();
}

VOID
SEnvConOutEnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  )
{
  PRINT_MODE  *PrintMode;

  PrintMode             = &mConOutProxy.PrintMode;

  PrintMode->PageBreak  = TRUE;
  PrintMode->OmitPrint  = FALSE;
  PrintMode->InitRow    = StartRow;
  PrintMode->AutoWrap   = AutoWrap;
  PrintMode->Row        = StartRow;
  PrintMode->Column     = ST->ConOut->Mode->CursorColumn;

  ST->ConOut->QueryMode (
                ST->ConOut,
                ST->ConOut->Mode->Mode,
                &PrintMode->MaxColumn,
                &PrintMode->MaxRow
                );
}

VOID
SEnvConOutDisablePageBreak (
  IN VOID
  )
{
  mConOutProxy.PrintMode.PageBreak  = FALSE;
  mConOutProxy.PrintMode.OmitPrint  = FALSE;
}

BOOLEAN
SEnvConOutGetPageBreak (
  IN VOID
  )
{
  return mConOutProxy.PrintMode.PageBreak;
}

STATIC
VOID
SEnvConOutSetOutputPause (
  IN BOOLEAN    Pause
  )
{
  ConOutSetOutputPause (&mConOutProxy, Pause);
}

STATIC
BOOLEAN
SEnvConOutGetOutputPause (
  IN VOID
  )
{
  return ConOutGetOutputPause (&mConOutProxy);
}

VOID
SEnvSetPrintOmit (
  IN BOOLEAN    OmitPrint
  )
{
  EFI_TPL Tpl;

  Tpl = BS->RaiseTPL (EFI_TPL_NOTIFY);
  mConOutProxy.PrintMode.OmitPrint = OmitPrint;
  BS->RestoreTPL (Tpl);
}

BOOLEAN
SEnvGetPrintOmit (
  IN VOID
  )
{
  return mConOutProxy.PrintMode.OmitPrint;
}
//
// Private worker functions
//
STATIC
VOID
ConInProxyOpen (
  IN     EFI_HANDLE                   ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL  **ConIn
  )
{
  EFI_STATUS  Status;

  if (mConInProxy.OpenedCount) {
    mConInProxy.OpenedCount++;
    return ;
  }

  mConInProxy.SavedConIn = *ConIn;
  Status = BS->ReinstallProtocolInterface (
                ConInHandle,
                &gEfiSimpleTextInProtocolGuid,
                *ConIn,
                &mConInProxy.ProxyConIn
                );

  if (!EFI_ERROR (Status)) {
    mConInProxy.KeyBuffer.Count     = 0;
    mConInProxy.KeyBuffer.StartPos  = 0;
    mConInProxy.KeyBuffer.EndPos    = 0;

    mConInProxy.KeyBuffer.Buffer = (EFI_INPUT_KEY *) AllocateZeroPool (KEY_BUFFER_MAX_COUNT * sizeof (EFI_INPUT_KEY));

    ASSERT (mConInProxy.KeyBuffer.Buffer);

    //
    // Create a timer to poll key periodically
    //
    Status = BS->CreateEvent (
                  EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_NOTIFY,
                  ProxyConInCheckKey,
                  &mConInProxy,
                  &mConInProxy.ConInEvent
                  );

    ASSERT (!EFI_ERROR (Status));

    Status = BS->SetTimer (
                  mConInProxy.ConInEvent,
                  TimerPeriodic,
                  500000
                  );

    ASSERT (!EFI_ERROR (Status));

    Status = BS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  ProxyConInWaitForKey,
                  &mConInProxy,
                  &mConInProxy.ProxyConIn.WaitForKey
                  );

    ASSERT (!EFI_ERROR (Status));
    *ConIn                  = &mConInProxy.ProxyConIn;

    mConInProxy.InputState  = INPUT_STATE_DEFAULT;

    Status = BS->CreateEvent (
                  EFI_EVENT_TIMER,
                  EFI_TPL_CALLBACK,
                  NULL,
                  NULL,
                  &mConInProxy.TwoSecondTimeOut
                  );
  }

  mConInProxy.OpenedCount++;
}

STATIC
VOID
ConInProxyClose (
  IN     EFI_HANDLE                ConInHandle,
  OUT EFI_SIMPLE_TEXT_IN_PROTOCOL  **ConIn
  )
{
  EFI_STATUS  Status;

  if (mConInProxy.SavedConIn == NULL) {
    return ;
  }

  mConInProxy.OpenedCount--;
  if (mConInProxy.OpenedCount > 0) {
    return ;
  }

  Status = BS->ReinstallProtocolInterface (
                ConInHandle,
                &gEfiSimpleTextInProtocolGuid,
                *ConIn,
                mConInProxy.SavedConIn
                );
  if (!EFI_ERROR (Status)) {
    *ConIn                  = mConInProxy.SavedConIn;
    mConInProxy.SavedConIn  = NULL;
    BS->SetTimer (
          mConInProxy.ConInEvent,
          TimerCancel,
          100000
          );
    BS->CloseEvent (mConInProxy.ConInEvent);
    BS->CloseEvent (mConInProxy.ProxyConIn.WaitForKey);
    FreePool (mConInProxy.KeyBuffer.Buffer);
    mConInProxy.KeyBuffer.Buffer = NULL;
    BS->CloseEvent (mConInProxy.TwoSecondTimeOut);
  }
}

STATIC
VOID
ConOutProxyOpen (
  IN     EFI_HANDLE                    ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL  **ConOut
  )
{
  EFI_STATUS  Status;

  if (mConOutProxy.OpenedCount) {
    mConOutProxy.OpenedCount++;
    return ;
  }
  //
  // The mode of proxy ConOut is just pointed to the original ConOut's.
  //
  mConOutProxy.ProxyConOut.Mode = (*ConOut)->Mode;
  mConOutProxy.SavedConOut      = *ConOut;
  Status = BS->ReinstallProtocolInterface (
                ConOutHandle,
                &gEfiSimpleTextOutProtocolGuid,
                *ConOut,
                &mConOutProxy.ProxyConOut
                );
  if (!EFI_ERROR (Status)) {
    *ConOut = &mConOutProxy.ProxyConOut;
    ConOutHistoryCreate ();
  }

  mConOutProxy.OpenedCount++;
}

STATIC
VOID
ConOutProxyClose (
  IN     EFI_HANDLE                 ConOutHandle,
  OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL  **ConOut
  )
{
  EFI_STATUS  Status;

  if (mConOutProxy.SavedConOut == NULL) {
    return ;
  }

  mConOutProxy.OpenedCount--;
  if (mConOutProxy.OpenedCount) {
    return ;
  }

  Status = BS->ReinstallProtocolInterface (
                ConOutHandle,
                &gEfiSimpleTextOutProtocolGuid,
                *ConOut,
                mConOutProxy.SavedConOut
                );
  if (!EFI_ERROR (Status)) {
    *ConOut                   = mConOutProxy.SavedConOut;
    mConOutProxy.SavedConOut  = NULL;
    ConOutHistoryDestory ();
  }

}

STATIC
EFI_STATUS
ConOutHistoryCreate (
  VOID
  )
{
  EFI_STATUS  Status;

  mConOutProxy.HistoryScreens = CONSOLE_OUTPUT_HISTORY_SCREENS;

  //
  // Create the history buffer by the current mode of the original console output.
  //
  Status = ConOutHistorySetMode (
            &mConOutProxy,
            mConOutProxy.SavedConOut->Mode->Mode
            );

  if (!EFI_ERROR (Status)) {
    mConOutProxy.HistoryEnabled = TRUE;
    //
    // Sync current settings with the original ConOut, in case the original ConOut
    // has not been in the initial state before creating the history buffer.
    //
    CopyMem (
      &mConOutProxy.HistoryMode,
      mConOutProxy.SavedConOut->Mode,
      sizeof (EFI_SIMPLE_TEXT_OUTPUT_MODE)
      );
  }

  return Status;
}

STATIC
VOID
ConOutHistoryDestory (
  VOID
  )
{
  mConOutProxy.HistoryValid             = FALSE;
  mConOutProxy.HistoryEnabled           = FALSE;
  mConOutProxy.HistoryColumnsPerScreen  = 0;
  mConOutProxy.HistoryRowsPerScreen     = 0;
  mConOutProxy.HistoryScreens           = 0;
  mConOutProxy.HistoryColumns           = 0;
  mConOutProxy.HistoryRows              = 0;
  mConOutProxy.HistoryVisibleStartRow   = 0;
  mConOutProxy.HistoryOriginalStartRow  = 0;

  FreePool (mConOutProxy.HistoryScreen);
  mConOutProxy.HistoryScreen = NULL;

  FreePool (mConOutProxy.HistoryAttributes);
  mConOutProxy.HistoryAttributes = NULL;

  ZeroMem (&mConOutProxy.HistoryMode, sizeof (EFI_SIMPLE_TEXT_OUTPUT_MODE));
}

STATIC
EFI_STATUS
ConOutHistoryUpdateVisibleScreen (
  VOID
  )
/*++
  Routine Description:
    Update the original console output device with the visible screen.
 
  Arguments:

  Returns:
    EFI_SUCCESS - The request is valid.
    other       - Return status of TextOut->OutputString ()
               
--*/
{
  EFI_STATUS                  Status;
  EFI_STATUS                  ReturnStatus;
  UINTN                       VisibleScreenOffset;
  UINTN                       Row;
  UINTN                       Column;
  UINTN                       MaxColumn;
  UINTN                       CurrentColumn;
  CHAR16                      *Screen;
  CHAR16                      *Str;
  CHAR16                      *ScreenStart;
  CHAR16                      RememberTempNull;
  INT32                       CurrentAttribute;
  INT32                       *Attributes;
  EFI_SIMPLE_TEXT_OUTPUT_MODE *Mode;
  CHAR16                      RememberLastChar;

  RememberLastChar  = 0x00;
  Mode              = &mConOutProxy.HistoryMode;

  mConOutProxy.SavedConOut->EnableCursor (mConOutProxy.SavedConOut, FALSE);
  mConOutProxy.SavedConOut->SetCursorPosition (mConOutProxy.SavedConOut, 0, 0);

  VisibleScreenOffset = mConOutProxy.HistoryColumns * mConOutProxy.HistoryVisibleStartRow;

  ReturnStatus        = EFI_SUCCESS;
  Screen              = &mConOutProxy.HistoryScreen[VisibleScreenOffset + mConOutProxy.HistoryVisibleStartRow];
  Attributes          = &mConOutProxy.HistoryAttributes[VisibleScreenOffset];
  MaxColumn           = mConOutProxy.HistoryColumns;
  for (Row = 0; Row < mConOutProxy.HistoryRowsPerScreen; Row++, Screen += (MaxColumn + 1), Attributes += MaxColumn) {

    if (Row == (mConOutProxy.HistoryRowsPerScreen - 1)) {
      //
      // Don't ever sync the last character as it will scroll the screen
      //
      RememberLastChar      = Screen[MaxColumn - 1];
      Screen[MaxColumn - 1] = 0x00;
    }

    Column = 0;
    while (Column < MaxColumn) {
      if (Screen[Column]) {
        CurrentAttribute  = Attributes[Column];
        CurrentColumn     = Column;
        ScreenStart       = &Screen[Column];

        //
        // The line end is alway 0x0. So Column should be less than MaxColumn
        // It should be still in the same row
        //
        for (Str = ScreenStart, RememberTempNull = 0; *Str != 0; Str++, Column++) {
          if (Attributes[Column] != CurrentAttribute) {
            RememberTempNull  = *Str;
            *Str              = 0;
            break;
          }
        }

        mConOutProxy.SavedConOut->SetAttribute (
                                    mConOutProxy.SavedConOut,
                                    CurrentAttribute
                                    );
        mConOutProxy.SavedConOut->SetCursorPosition (
                                    mConOutProxy.SavedConOut,
                                    CurrentColumn,
                                    Row
                                    );

        Status = mConOutProxy.SavedConOut->OutputString (
                                            mConOutProxy.SavedConOut,
                                            ScreenStart
                                            );

        if (EFI_ERROR (Status)) {
          ReturnStatus = Status;
        }

        if (RememberTempNull) {
          *Str = RememberTempNull;
          //
          // Make sure we process the character with the new attributes
          //
          Column--;
        }
      }

      Column++;
    }

    if (RememberLastChar) {
      //
      // make sure we restore the char which is temporary modified to aVOID sync right bottom screen.
      //
      Screen[MaxColumn - 1] = RememberLastChar;
      RememberLastChar      = 0x00;
    }
  }
  //
  // If restoring the original screen, restore the device's Attribute, Cursor
  // enable state and location.
  // Otherwise the history screen is being scrolled, do not restore them.
  //
  if (mConOutProxy.HistoryVisibleStartRow == mConOutProxy.HistoryOriginalStartRow) {
    mConOutProxy.SavedConOut->SetAttribute (
                                mConOutProxy.SavedConOut,
                                Mode->Attribute
                                );
    mConOutProxy.SavedConOut->SetCursorPosition (
                                mConOutProxy.SavedConOut,
                                Mode->CursorColumn,
                                Mode->CursorRow - mConOutProxy.HistoryOriginalStartRow
                                );
    Status = mConOutProxy.SavedConOut->EnableCursor (
                                        mConOutProxy.SavedConOut,
                                        Mode->CursorVisible
                                        );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}
//
// ConSplitter TextIn member functions
//
EFI_STATUS
EFIAPI
ProxyConInReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES            - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
{
  EFI_STATUS                Status;
  CON_IN_PROXY_PRIVATE_DATA *Private;

  Private = CON_IN_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConIn
  //
  Status = Private->SavedConIn->Reset (Private->SavedConIn, ExtendedVerification);

  ProxyConInResetKeyBuf ();

  return Status;
}

EFI_STATUS
EFIAPI
ProxyConInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This,
  OUT EFI_INPUT_KEY                 *Key
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This   - Protocol instance pointer.
    Key    - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES        - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to 
                        hardware errors.

--*/
{
  EFI_STATUS                Status;
  CON_IN_PROXY_PRIVATE_DATA *Private;

  Private = CON_IN_PROXY_PRIVATE_DATA_FROM_THIS (This);

  Status  = ProxyConInReadKeyBuf (Private, Key);

  //
  // If no records in buffer, call console driver directly to speed the detection.
  //
  if (EFI_ERROR (Status)) {
    Status = Private->SavedConIn->ReadKeyStroke (Private->SavedConIn, Key);
  }

  if (Status == EFI_SUCCESS) {
    Status = ProxyConInFlashState (Private, Key);
  }
  //
  // For non-execution status, no filters will be applied.
  //
  if (Status == EFI_SUCCESS) {
    Status = ProxyConInKeyFilter (Private, Key);
  }

  return Status;
}

VOID
EFIAPI
ProxyConInCheckKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
/*++

Routine Description:
  Event notification function for SIMPLE_TEXT_IN.WaitForKey event
  Signal the event if there is key available
  If the ConIn is password locked then return.  


Arguments:
  Event   - The Event assoicated with callback.
  Context - Context registered when Event was created.  

Returns:

  None

--*/
{
  EFI_STATUS                Status;
  CON_IN_PROXY_PRIVATE_DATA *Private;
  EFI_INPUT_KEY             Key;

  Private = (CON_IN_PROXY_PRIVATE_DATA *) Context;

  //
  // Forward the request to the original ConIn
  //
  Status = BS->CheckEvent (Private->SavedConIn->WaitForKey);
  if (Status == EFI_SUCCESS) {
    Status = Private->SavedConIn->ReadKeyStroke (Private->SavedConIn, &Key);
    if (!EFI_ERROR (Status)) {
      Status = ProxyConInFlashState (Private, &Key);
      //
      // Apply filter to the key
      //
      Status = ProxyConInKeyFilter (Private, &Key);
      if (!EFI_ERROR (Status)) {
        //
        // No filters hit the key, so record it into buffer
        //
        ProxyConInAddKeyBuf (Private, Key);
      }
    }
  }

  return ;
}

VOID
EFIAPI
ProxyConInWaitForKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
/*++

Routine Description:
  Event notification function for SIMPLE_TEXT_IN.WaitForKey event
  Signal the event if there is key available
  If the ConIn is password locked then return.  

Arguments:
  Event   - The Event assoicated with callback.
  Context - Context registered when Event was created.

Returns:
  None

--*/
{
  EFI_STATUS                Status;
  CON_IN_PROXY_PRIVATE_DATA *Private;

  Private = (CON_IN_PROXY_PRIVATE_DATA *) Context;

  //
  // if any physical console input device has key input, signal the event.
  //
  if (Private->KeyBuffer.Count > 0) {

    BS->SignalEvent (Event);

  } else {

    Status = BS->CheckEvent (Private->SavedConIn->WaitForKey);
    if (Status == EFI_SUCCESS) {
      BS->SignalEvent (Event);
    }
  }
}

VOID
EFIAPI
SEnvSetKeyFilter (
  IN UINT32      KeyFilter
  )
/*++

Routine Description:
  Set curretn ky filter setting
  
Arguments:  
  KeyFilter   - The new key filter to set
               
Returns:    
--*/
{
  mConInProxy.KeyFilter = KeyFilter;
}

UINT32
EFIAPI
SEnvGetKeyFilter (
  IN VOID
  )
/*++

Routine Description:
  Get curretn ky filter setting
  
Arguments:  

  None
               
Returns:    
--*/
{
  return mConInProxy.KeyFilter;
}

EFI_STATUS
ProxyConInFlashState (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  IN  EFI_INPUT_KEY                  *Key
  )
{
  EFI_STATUS  Status;
  EFI_STATUS  TimerStatus;

  TimerStatus = BS->CheckEvent (mConInProxy.TwoSecondTimeOut);

  if (!EFI_ERROR (TimerStatus)) {
    mConInProxy.InputState = INPUT_STATE_DEFAULT;
    BS->SetTimer (
          mConInProxy.TwoSecondTimeOut,
          TimerCancel,
          0
          );
  } else if (Key->UnicodeChar == 0 && Key->ScanCode == SCAN_ESC) {
    mConInProxy.InputState = INPUT_STATE_ESC;
    Status = BS->SetTimer (
                  mConInProxy.TwoSecondTimeOut,
                  TimerRelative,
                  (UINT64) 20000000
                  );
    mConInProxy.InputState = INPUT_STATE_ESC;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ProxyConInKeyFilter (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  IN  EFI_INPUT_KEY                  *Key
  )
/*++

Routine Description:
  Filt some special functin key in proxy console
  
Arguments:

  Private    - The pointer to the private data of console proxy                

  Key        - The key structure to be added in
               
Returns:   
  EFI_NOT_READY  - The key has been filtered, and won't be added into buffer
  EFI_SUCCESS    - The key will be added into buffer
--*/
{

  if (SEnvConOutGetOutputPause ()) {
    return EFI_SUCCESS;
  }

  if (mConInProxy.KeyFilter & EFI_OUTPUT_PAUSE) {
    if (OUTPUT_PAUSE_KEY(Key)) {
      SEnvConOutSetOutputPause (TRUE);
      ProxyConInResetKeyBuf ();
      return EFI_NOT_READY;
    }
  }

  if ((mConInProxy.KeyFilter & EFI_EXECUTION_BREAK)) {
    if (OUTPUT_BREAK_KEY(Key)) {
      SEnvEnableExecutionBreak ();
      return EFI_SUCCESS;
    }
  }

  return EFI_SUCCESS;
}
//
// Proxy console output protocol implementation
//
EFI_STATUS
EFIAPI
ProxyConOutReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       ExtendedVerification
  )
/*++

  Routine Description:
    Reset the text output device hardware and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform more exhaustive verfication 
                           operation of the device during reset.

  Returns:
    EFI_SUCCESS       - The text output device was reset.
    EFI_DEVICE_ERROR  - The text output device is not functioning correctly and
                        could not be reset.

--*/
{
  EFI_STATUS                  Status;
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  Status = Private->SavedConOut->Reset (Private->SavedConOut, ExtendedVerification);

  //
  // Record console output history
  //
  //
  // Always do mode change, even if the history buffer is disabled or invalid.
  // And it's a chance to make the history buffer valid.
  //
  if (!EFI_ERROR (Status)) {
    ConOutHistorySetMode (Private, 0);
  }

  return Status;
}

EFI_STATUS
EFIAPI
ProxyConOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
/*++

  Routine Description:
    Write a Unicode string to the output device.

  Arguments:
    This    - Protocol instance pointer.
    WString - The NULL-terminated Unicode string to be displayed on the output
              device(s). All output devices must also support the Unicode 
              drawing defined in this file.

  Returns:
    EFI_SUCCESS       - The string was output to the device.
    EFI_DEVICE_ERROR  - The device reported an error while attempting to output
                         the text.
    EFI_UNSUPPORTED   - The output device's mode is not currently in a 
                         defined text mode.
    EFI_WARN_UNKNOWN_GLYPH - This warning code indicates that some of the 
                              characters in the Unicode string could not be 
                              rendered and were skipped.

--*/
{
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Output string upon the settings of the print mode.
  //
  if (Private->PrintMode.OmitPrint) {
    return EFI_SUCCESS;
  }

  if (Private->PrintMode.PageBreak) {
    return ConOutOutputStringWithPageBreak (Private, WString);
  } else {
    return ConOutOutputString (Private, WString);
  }
}

EFI_STATUS
EFIAPI
ProxyConOutTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  )
/*++

  Routine Description:
    Verifies that all characters in a Unicode string can be output to the 
    target device.

  Arguments:
    This    - Protocol instance pointer.
    WString - The NULL-terminated Unicode string to be examined for the output
               device(s).

  Returns:
    EFI_SUCCESS     - The device(s) are capable of rendering the output string.
    EFI_UNSUPPORTED - Some of the characters in the Unicode string cannot be 
                       rendered by one or more of the output devices mapped 
                       by the EFI handle.

--*/
{
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  return Private->SavedConOut->TestString (Private->SavedConOut, WString);
}

EFI_STATUS
EFIAPI
ProxyConOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber,
  OUT UINTN                         *Columns,
  OUT UINTN                         *Rows
  )
/*++

  Routine Description:
    Returns information for an available text mode that the output device(s)
    supports.

  Arguments:
    This       - Protocol instance pointer.
    ModeNumber - The mode number to return information on.
    Columns, Rows - Returns the geometry of the text output device for the
                    requested ModeNumber.

  Returns:
    EFI_SUCCESS      - The requested mode information was returned.
    EFI_DEVICE_ERROR - The device had an error and could not 
                       complete the request.
    EFI_UNSUPPORTED - The mode number was not valid.

--*/
{
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  return Private->SavedConOut->QueryMode (
                                Private->SavedConOut,
                                ModeNumber,
                                Columns,
                                Rows
                                );
}

EFI_STATUS
EFIAPI
ProxyConOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber
  )
/*++

  Routine Description:
    Sets the output device(s) to a specified mode.

  Arguments:
    This       - Protocol instance pointer.
    ModeNumber - The mode number to set.

  Returns:
    EFI_SUCCESS      - The requested text mode was set.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The mode number was not valid.

--*/
{
  EFI_STATUS                  Status;
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  Status = Private->SavedConOut->SetMode (Private->SavedConOut, ModeNumber);

  //
  // Record console output history
  //
  //
  // Always do mode change, even if the history buffer is disabled or invalid.
  // And it's a chance to make the history buffer valid.
  //
  if (!EFI_ERROR (Status)) {
    ConOutHistorySetMode (Private, ModeNumber);
  }

  return Status;
}

EFI_STATUS
EFIAPI
ProxyConOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Attribute
  )
/*++

  Routine Description:
    Sets the background and foreground colors for the OutputString () and
    ClearScreen () functions.

  Arguments:
    This      - Protocol instance pointer.
    Attribute - The attribute to set. Bits 0..3 are the foreground color, and
                bits 4..6 are the background color. All other bits are undefined
                and must be zero. The valid Attributes are defined in this file.

  Returns:
    EFI_SUCCESS      - The attribute was set.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The attribute requested is not defined.

--*/
{
  EFI_STATUS                  Status;
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  Status = Private->SavedConOut->SetAttribute (Private->SavedConOut, Attribute);

  //
  // Record console output history
  //
  if (!EFI_ERROR (Status) && Private->HistoryValid && Private->HistoryEnabled) {
    ConOutHistorySetAttribute (Private, Attribute);
  }

  return Status;
}

EFI_STATUS
EFIAPI
ProxyConOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  )
/*++

  Routine Description:
    Clears the output device(s) display to the currently selected background 
    color.

  Arguments:
    This      - Protocol instance pointer.

  Returns:
    EFI_SUCCESS      - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode.

--*/
{
  EFI_STATUS                  Status;
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  Status = Private->SavedConOut->ClearScreen (Private->SavedConOut);

  //
  // Record console output history
  //
  if (!EFI_ERROR (Status) && Private->HistoryValid && Private->HistoryEnabled) {
    ConOutHistoryClearScreen (Private);
  }

  return Status;
}

EFI_STATUS
EFIAPI
ProxyConOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  )
/*++

  Routine Description:
    Sets the current coordinates of the cursor position

  Arguments:
    This        - Protocol instance pointer.
    Column, Row - the position to set the cursor to. Must be greater than or
                  equal to zero and less than the number of columns and rows
                  by QueryMode ().

  Returns:
    EFI_SUCCESS      - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and 
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode, or the 
                       cursor position is invalid for the current mode.

--*/
{
  EFI_STATUS                  Status;
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  Status = Private->SavedConOut->SetCursorPosition (
                                  Private->SavedConOut,
                                  Column,
                                  Row
                                  );

  //
  // Record console output history
  //
  if (!EFI_ERROR (Status) && Private->HistoryValid && Private->HistoryEnabled) {
    ConOutHistorySetCursorPosition (Private, Column, Row);
  }

  return Status;
}

EFI_STATUS
EFIAPI
ProxyConOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       Visible
  )
/*++

  Routine Description:
    Makes the cursor visible or invisible

  Arguments:
    This    - Protocol instance pointer.
    Visible - If TRUE, the cursor is set to be visible. If FALSE, the cursor is
              set to be invisible.

  Returns:
    EFI_SUCCESS      - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and could not complete the 
                        request, or the device does not support changing
                        the cursor mode.
    EFI_UNSUPPORTED - The output device is not in a valid text mode.

--*/
{
  EFI_STATUS                  Status;
  CON_OUT_PROXY_PRIVATE_DATA  *Private;

  Private = CON_OUT_PROXY_PRIVATE_DATA_FROM_THIS (This);

  //
  // Forward the request to the original ConOut
  //
  Status = Private->SavedConOut->EnableCursor (Private->SavedConOut, Visible);

  //
  // Record console output history
  //
  if (!EFI_ERROR (Status) && Private->HistoryValid && Private->HistoryEnabled) {
    ConOutHistoryEnableCursor (Private, Visible);
  }

  return Status;
}

STATIC
EFI_STATUS
ConOutHistoryOutputString (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  CHAR16                          *WString
  )
/*++

  Routine Description:
    Write a Unicode string to the output history buffer.

  Arguments:
    Private - Private data instance pointer.
    WString - The NULL-terminated Unicode string to be displayed on the output
              device(s). All output devices must also support the Unicode 
              drawing defined in this file.

  Returns:
    EFI_SUCCESS       - The string was output to the device.
--*/
{
  UINTN                       SizeScreen;
  UINTN                       SizeAttribute;
  UINTN                       Index;
  EFI_SIMPLE_TEXT_OUTPUT_MODE *Mode;
  CHAR16                      *Screen;
  INT32                       *Attribute;

  Mode = &Private->HistoryMode;

  while (*WString) {

    if (*WString == CHAR_BACKSPACE) {
      //
      // If the cursor is not at the left edge of the history screen,
      // then move the cursor left one column.
      //
      if (Mode->CursorColumn > 0) {
        Mode->CursorColumn--;
      }

      WString++;

    } else if (*WString == CHAR_LINEFEED) {
      //
      // If the cursor is at the bottom of the history screen buffer, then scroll
      // the history screen up one row, and do not update the cursor position.
      // Otherwise, move the cursor down one row. And if the cursor is at the
      // bottom of both the visible screen and the original screen, then also move
      // both the visible screen and the original screen down one row.
      //
      if (Mode->CursorRow == (INT32) (Private->HistoryRows - 1)) {
        //
        // Scroll History Screen Buffer Up One Row
        //
        SizeAttribute = Private->HistoryColumns * (Private->HistoryRows - 1);
        CopyMem (
          Private->HistoryAttributes,
          Private->HistoryAttributes + Private->HistoryColumns,
          SizeAttribute * sizeof (INT32)
          );

        //
        // Each row has an ending CHAR_NULL. So one more character each line
        // for HistoryScreen than HistoryAttributes.
        //
        SizeScreen = SizeAttribute + (Private->HistoryRows - 1);
        CopyMem (
          Private->HistoryScreen,
          Private->HistoryScreen + Private->HistoryColumns + 1,
          SizeScreen * sizeof (CHAR16)
          );

        //
        // Print Blank Line at last line
        //
        Screen    = Private->HistoryScreen + SizeScreen;
        Attribute = Private->HistoryAttributes + SizeAttribute;

        for (Index = 0; Index < Private->HistoryColumns; Index++, Screen++, Attribute++) {
          *Screen     = ' ';
          *Attribute  = Mode->Attribute;
        }
      } else {
        //
        // The visible screen should be equivalent to the original screen, unless
        // the history screen buffer is being scrolled. Here the history screen
        // buffer should not be being scrolled. So if the cursor is at the bottom
        // of the original screen, then move both the visible screen and the original
        // screen down one row.
        //
        ASSERT (Private->HistoryOriginalStartRow == Private->HistoryVisibleStartRow);
        if (Mode->CursorRow == (INT32) (Private->HistoryOriginalStartRow + Private->HistoryRowsPerScreen - 1)) {
          Private->HistoryOriginalStartRow++;
          Private->HistoryVisibleStartRow++;
        }

        Mode->CursorRow++;
      }

      WString++;
    } else if (*WString == CHAR_CARRIAGE_RETURN) {
      //
      // Move the cursor to the beginning of the current row.
      //
      Mode->CursorColumn = 0;
      WString++;
    } else {
      //
      // Print the character at the current cursor position and
      // move the cursor right one column. If this moves the cursor
      // past the right edge of the display, then the line should wrap to
      // the beginning of the next line. This is equivalent to inserting
      // a CR and an LF. Note that if the cursor is at the bottom of the
      // display, and the line wraps, then the display will be scrolled
      // one line.
      //
      Index = Mode->CursorRow * Private->HistoryColumns + Mode->CursorColumn;

      while (Mode->CursorColumn < (INT32) Private->HistoryColumns) {
        if (*WString == CHAR_NULL) {
          break;
        }

        if (*WString == CHAR_BACKSPACE) {
          break;
        }

        if (*WString == CHAR_LINEFEED) {
          break;
        }

        if (*WString == CHAR_CARRIAGE_RETURN) {
          break;
        }

        Private->HistoryScreen[Index + Mode->CursorRow] = *WString;
        Private->HistoryAttributes[Index]               = Mode->Attribute;
        Index++;
        WString++;
        Mode->CursorColumn++;
      }
      //
      // At the end of line, output carriage return and line feed
      //
      if (Mode->CursorColumn >= (INT32) Private->HistoryColumns) {
        ConOutHistoryOutputString (Private, mCrLfString);
      }
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConOutHistorySetMode (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private,
  IN  UINTN                         ModeNumber
  )
/*++

  Routine Description:
    Sets the output history buffer to a specified mode.
    This is an internal function.

  Arguments:
    Private    - Private data instance pointer.
    ModeNumber - The mode number to set.

  Returns:
    EFI_SUCCESS          - The requested text mode was set.
    EFI_DEVICE_ERROR     - The device had an error and 
                          could not complete the request.
    EFI_UNSUPPORTED      - The mode number was not valid.
    EFI_OUT_OF_RESOURCES - There is not enough memory for the requested mode.
--*/
{
  EFI_STATUS  Status;
  UINTN       Size;
  UINTN       Rows;
  UINTN       Columns;

  //
  // No extra check for ModeNumber here, as it has been checked in
  // caller. And mode 0 should always be supported.
  //
  Status = Private->SavedConOut->QueryMode (
                                  Private->SavedConOut,
                                  ModeNumber,
                                  &Columns,
                                  &Rows
                                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // If the mode is changed, reallocate and re-setup the history screen buffer.
  //
  if (Private->HistoryColumnsPerScreen != Columns || Private->HistoryRowsPerScreen != Rows) {
    //
    // Set history buffer to invalid before re-setup
    //
    Private->HistoryValid = FALSE;

    if (Private->HistoryScreen) {
      FreePool (Private->HistoryScreen);
    }

    Size                    = (Rows * (Columns + 1) * Private->HistoryScreens) * sizeof (CHAR16);
    Private->HistoryScreen  = AllocateZeroPool (Size);
    if (Private->HistoryScreen == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    if (Private->HistoryAttributes) {
      FreePool (Private->HistoryAttributes);
    }

    Size                        = Rows * Columns * Private->HistoryScreens * sizeof (INT32);
    Private->HistoryAttributes  = AllocateZeroPool (Size);
    if (Private->HistoryAttributes == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Private->HistoryColumnsPerScreen  = Columns;
    Private->HistoryRowsPerScreen     = Rows;
    Private->HistoryColumns           = Columns;
    Private->HistoryRows              = Rows * Private->HistoryScreens;
    Private->HistoryVisibleStartRow   = 0;
    Private->HistoryOriginalStartRow  = 0;
    Private->HistoryMode.Mode         = (INT32) ModeNumber;

    //
    // Set history buffer to invalid after re-setup
    //
    Private->HistoryValid = TRUE;
  }

  ConOutHistoryClearScreen (Private);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConOutHistorySetAttribute (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  UINTN                           Attribute
  )
/*++

  Routine Description:
    Sets the background and foreground colors for the OutputString () and
    ClearScreen () functions.
    This is an internal function.

  Arguments:
    Private      - Private data instance pointer.
    Attribute - The attribute to set. Bits 0..3 are the foreground color, and
                bits 4..6 are the background color. All other bits are undefined
                and must be zero. The valid Attributes are defined in this file.

  Returns:
    EFI_SUCCESS      - The attribute was set.

--*/
{
  Private->HistoryMode.Attribute = (INT32) Attribute;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConOutHistoryClearScreen (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private
  )
/*++

  Routine Description:
    Clears the output history buffer to the currently selected background color.
    This is an internal function.

  Arguments:
    Private      - Private data instance pointer.

  Returns:
    EFI_SUCCESS      - The operation completed successfully.

--*/
{
  UINTN   Row;
  UINTN   Column;
  UINTN   OriginalScreenOffset;
  CHAR16  *Screen;
  INT32   *Attributes;
  INT32   CurrentAttribute;

  //
  // Clear the original screen in the history screen buffer, including the unused buffer.
  // The screen is filled with spaces.
  // The attributes are all synced with the current Simple Text Out Attribute.
  //
  OriginalScreenOffset  = Private->HistoryColumns * Private->HistoryOriginalStartRow;
  Screen                = &Private->HistoryScreen[OriginalScreenOffset + Private->HistoryOriginalStartRow];
  Attributes            = &Private->HistoryAttributes[OriginalScreenOffset];
  CurrentAttribute      = Private->SavedConOut->Mode->Attribute;

  for (Row = Private->HistoryOriginalStartRow; Row < Private->HistoryRows; Row++) {
    for (Column = 0; Column < Private->HistoryColumns; Column++, Screen++, Attributes++) {
      *Screen     = ' ';
      *Attributes = CurrentAttribute;
    }
    //
    // Each line of the screen has a NULL on the end so we must skip over it
    //
    Screen++;
  }

  return ConOutHistorySetCursorPosition (Private, 0, 0);
}

STATIC
EFI_STATUS
ConOutHistorySetCursorPosition (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  )
/*++

  Routine Description:
    Sets the current coordinates of the cursor position for the output history buffer.
    This is an internal function.

  Arguments:
    Private     - Private data instance pointer.
    Column, Row - The position to set the cursor to. Must be greater than or
                  equal to zero and less than the number of columns and rows
                  by QueryMode ().

  Returns:
    EFI_SUCCESS      - The operation completed successfully.

--*/
{
  //
  // No need to do extra check here as whether (Column, Row) is valid has
  // been checked in caller. And (0, 0) should always be supported.
  //
  //
  // Should add the start position of the original screen.
  //
  Private->HistoryMode.CursorColumn = (INT32) Column;
  Private->HistoryMode.CursorRow    = (INT32) (Private->HistoryOriginalStartRow + Row);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConOutHistoryEnableCursor (
  IN  CON_OUT_PROXY_PRIVATE_DATA    *Private,
  IN  BOOLEAN                       Visible
  )
/*++

  Routine Description:
    Makes the cursor visible or invisible.
    This is an internal function.

  Arguments:
    Private - Private data instance pointer.
    Visible - If TRUE, the cursor is set to be visible. If FALSE, the cursor is
              set to be invisible.

  Returns:
    EFI_SUCCESS      - The operation completed successfully.

--*/
{
  Private->HistoryMode.CursorVisible = Visible;

  return EFI_SUCCESS;
}

VOID
ProxyConInResetKeyBuf (
  VOID
  )
/*++

Routine Description:
  Rest the key buffer in console proxy
  
Arguments:  
               
Returns:

--*/
{
  mConInProxy.KeyBuffer.Count     = 0;
  mConInProxy.KeyBuffer.StartPos  = 0;
  mConInProxy.KeyBuffer.EndPos    = 0;
}

EFI_STATUS
ProxyConInReadKeyBuf (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  OUT EFI_INPUT_KEY                  *Key
  )
/*++

Routine Description:
  Read a key from the key buffer in console proxy
  
Arguments:

  Private    - The pointer to the private data of console proxy                

  Key        - The pointer to the key structure to retrieve the key
               
Returns:  
  EFI_SUCCESS        - The keystroke information was returned.
  EFI_NOT_READY      - There was no keystroke data availiable.
  EFI_DEVICE_ERROR   - The keydtroke information was not returned due to 
                        hardware errors.
--*/
{
  //
  // No keys in buffer
  //
  if (Private->KeyBuffer.Count == 0) {
    return EFI_NOT_READY;
  }
  //
  // For synchronization, lock it
  //
  AcquireLock (&SEnvGuidLock);
  *Key = Private->KeyBuffer.Buffer[Private->KeyBuffer.StartPos++];
  Private->KeyBuffer.Count--;
  if (Private->KeyBuffer.StartPos >= KEY_BUFFER_MAX_COUNT) {
    Private->KeyBuffer.StartPos = 0;
  }

  ReleaseLock (&SEnvGuidLock);

  return EFI_SUCCESS;
}

VOID
ProxyConInAddKeyBuf (
  IN  CON_IN_PROXY_PRIVATE_DATA      *Private,
  IN  EFI_INPUT_KEY                  Key
  )
/*++

Routine Description:
  Add a key into the key buffer in console proxy
  
Arguments:

  Private    - The pointer to the private data of console proxy                

  Key        - The key structure to be added in
               
Returns:    
--*/
{
  //
  // Prevent timer process from modifying these variables
  // lock it at first
  //
  AcquireLock (&SEnvGuidLock);

  if (Private->KeyBuffer.Count < KEY_BUFFER_MAX_COUNT) {
    Private->KeyBuffer.Count++;
  } else {
    Private->KeyBuffer.StartPos++;
  }

  Private->KeyBuffer.Buffer[Private->KeyBuffer.EndPos++] = Key;

  if (Private->KeyBuffer.StartPos >= KEY_BUFFER_MAX_COUNT) {
    Private->KeyBuffer.StartPos = 0;
  }

  if (Private->KeyBuffer.EndPos >= KEY_BUFFER_MAX_COUNT) {
    Private->KeyBuffer.EndPos = 0;
  }

  ReleaseLock (&SEnvGuidLock);
}

STATIC
EFI_STATUS
ConOutOutputString (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  CHAR16                          *WString
  )
/*++

  Routine Description:
    Write a Unicode string to the output device.

  Arguments:
    Private - Private data instance pointer.
    WString - The NULL-terminated Unicode string to be displayed on the output
              device(s). All output devices must also support the Unicode 
              drawing defined in this file.

  Returns:
    EFI_SUCCESS       - The string was output to the device.
    EFI_DEVICE_ERROR  - The device reported an error while attempting to output
                         the text.
    EFI_UNSUPPORTED   - The output device's mode is not currently in a 
                         defined text mode.
    EFI_WARN_UNKNOWN_GLYPH - This warning code indicates that some of the 
                              characters in the Unicode string could not be 
                              rendered and were skipped.

--*/
{
  EFI_STATUS    Status;
  EFI_INPUT_KEY Key;

  //
  // Pause the output if desire
  //
  if (ConOutGetOutputPause (Private)) {
    WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
    ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
    ConOutSetOutputPause (Private, FALSE);
  }
  //
  // Forward the request to the original ConOut
  //
  Status = Private->SavedConOut->OutputString (Private->SavedConOut, WString);

  //
  // Record console output history
  //
  if (!EFI_ERROR (Status) && Private->HistoryValid && Private->HistoryEnabled) {
    ConOutHistoryOutputString (Private, WString);
  }

  return Status;
}

STATIC
EFI_STATUS
ConOutOutputStringWithPageBreak (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  CHAR16                          *WString
  )
/*++

  Routine Description:
    Write a Unicode string to the output device using page break mode.

  Arguments:
    Private - Private data instance pointer.
    WString - The NULL-terminated Unicode string to be displayed on the output
              device(s). All output devices must also support the Unicode 
              drawing defined in this file.

  Returns:
    EFI_SUCCESS       - The string was output to the device.
    EFI_DEVICE_ERROR  - The device reported an error while attempting to output
                         the text.
    EFI_UNSUPPORTED   - The output device's mode is not currently in a 
                         defined text mode.
    EFI_WARN_UNKNOWN_GLYPH - This warning code indicates that some of the 
                              characters in the Unicode string could not be 
                              rendered and were skipped.

--*/
{
  PRINT_MODE  *PrintMode;
  CHAR16      *LineStart;
  CHAR16      LineEndChar;

  PrintMode = &Private->PrintMode;

  LineStart = WString;
  while (*WString) {

    if (*WString == CHAR_BACKSPACE) {
      //
      // If the cursor is not at the left edge of the screen,
      // then move the cursor left one column.
      //
      if (PrintMode->Column > 0) {
        PrintMode->Column--;
      }
    } else if (*WString == CHAR_LINEFEED) {
      //
      // Output one line
      //
      LineEndChar     = *(WString + 1);
      *(WString + 1)  = 0;
      ConOutOutputString (Private, LineStart);
      //
      // Restore the line end char
      //
      *(WString + 1)  = LineEndChar;

      LineStart       = WString + 1;
      PrintMode->Row++;
      //
      // If reach one page, set page break.
      //
      if (PrintMode->Row == PrintMode->MaxRow) {
        if (ConOutSetPageBreak (Private)) {
          //
          // Omit the successive output
          //
          return EFI_SUCCESS;
        }
      }
    } else if (*WString == CHAR_CARRIAGE_RETURN) {
      //
      // Move the cursor to the beginning of the current row.
      //
      PrintMode->Column = 0;
    } else {
      //
      // move the cursor right one column. If this moves the cursor
      // past the right edge of the display, then the line should wrap to
      // the beginning of the next line.
      //
      PrintMode->Column++;

      if (PrintMode->Column == PrintMode->MaxColumn) {
        //
        // Output one line
        //
        LineEndChar     = *(WString + 1);
        *(WString + 1)  = 0;
        ConOutOutputString (Private, LineStart);
        //
        // Restore the line end char
        //
        *(WString + 1) = LineEndChar;

        //
        // The AutoWrap flag seems useless, since the console output always wrap
        // the text automatically.
        //
        if (PrintMode->AutoWrap) {
          ConOutOutputString (Private, mCrLfString);
        }

        LineStart         = WString + 1;
        PrintMode->Column = 0;
        PrintMode->Row++;
        //
        // If reach one page, set page break.
        //
        if (PrintMode->Row == PrintMode->MaxRow) {
          if (ConOutSetPageBreak (Private)) {
            //
            // Omit the successive output
            //
            return EFI_SUCCESS;
          }
        }
      }
    }

    WString++;
  }

  if (*LineStart != 0) {
    ConOutOutputString (Private, LineStart);
  }

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
ConOutSetPageBreak (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private
  )
{
  PRINT_MODE    *PrintMode;
  EFI_INPUT_KEY Key;
  CHAR16        Str[3];
  UINT32        KeyFilter;

  PrintMode = &Private->PrintMode;

  ConOutOutputString (Private, L"Press ENTER to continue, 'q' to exit:");

  //
  // Set KeyFilter to ensure not paused
  //
  KeyFilter             = mConInProxy.KeyFilter;
  mConInProxy.KeyFilter = EFI_OUTPUT_SCROLL;

  //
  // Wait for user input
  //
  Str[0]  = ' ';
  Str[1]  = 0;
  Str[2]  = 0;
  for (;;) {
    WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
    ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);

    //
    // handle control keys
    //
    if (Key.UnicodeChar == CHAR_NULL) {
      if (Key.ScanCode == SCAN_ESC) {
        ConOutOutputString (Private, mCrLfString);
        PrintMode->OmitPrint = TRUE;
        break;
      }

      continue;
    }

    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      ConOutOutputString (Private, mCrLfString);
      PrintMode->Column = 0;
      PrintMode->Row    = PrintMode->InitRow;
      break;
    }
    //
    // Echo input
    //
    Str[1] = Key.UnicodeChar;
    if (Str[1] == CHAR_BACKSPACE || Str[1] == CHAR_TAB) {
      continue;
    }

    ConOutOutputString (Private, Str);

    if ((Str[1] == L'q') || (Str[1] == L'Q')) {
      PrintMode->OmitPrint = TRUE;
    } else {
      PrintMode->OmitPrint = FALSE;
    }

    Str[0] = CHAR_BACKSPACE;
  }
  //
  // Set KeyFilter back
  //
  mConInProxy.KeyFilter = KeyFilter;

  return PrintMode->OmitPrint;
}

STATIC
VOID
ConOutSetOutputPause (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private,
  IN  BOOLEAN                         Pause
  )
{
  EFI_TPL Tpl;

  Tpl = BS->RaiseTPL (EFI_TPL_NOTIFY);
  Private->PrintMode.OutputPause = Pause;
  BS->RestoreTPL (Tpl);
}

STATIC
BOOLEAN
ConOutGetOutputPause (
  IN  CON_OUT_PROXY_PRIVATE_DATA      *Private
  )
{
  return Private->PrintMode.OutputPause;
}
