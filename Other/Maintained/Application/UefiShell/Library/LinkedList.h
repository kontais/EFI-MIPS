/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    LinkedList.h

Abstract:

    EFI link list macro's



Revision History

--*/
#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

//
// List entry - doubly linked list
//
typedef struct _EFI_LIST_ENTRY {
  struct _EFI_LIST_ENTRY  *Flink;
  struct _EFI_LIST_ENTRY  *Blink;
} EFI_LIST_ENTRY;

#define InitializeListHead(ListHead) \
  (ListHead)->Flink = ListHead; \
  (ListHead)->Blink = ListHead;

#define IsListEmpty(ListHead) ((ListHead)->Flink == (ListHead))

#define _RemoveEntryList(Entry) { \
    EFI_LIST_ENTRY  *_Blink, *_Flink; \
    _Flink        = (Entry)->Flink; \
    _Blink        = (Entry)->Blink; \
    _Blink->Flink = _Flink; \
    _Flink->Blink = _Blink; \
  }

#ifdef EFI_DEBUG
#define RemoveEntryList(Entry) \
  _RemoveEntryList (Entry); \
  (Entry)->Flink  = (EFI_LIST_ENTRY *) EFI_BAD_POINTER; \
  (Entry)->Blink  = (EFI_LIST_ENTRY *) EFI_BAD_POINTER;
#else
#define RemoveEntryList(Entry)  _RemoveEntryList (Entry);
#endif

#define InsertTailList(ListHead, Entry) { \
    EFI_LIST_ENTRY  *_ListHead, *_Blink; \
    _ListHead         = (ListHead); \
    _Blink            = _ListHead->Blink; \
    (Entry)->Flink    = _ListHead; \
    (Entry)->Blink    = _Blink; \
    _Blink->Flink     = (Entry); \
    _ListHead->Blink  = (Entry); \
  }

#define InsertHeadList(ListHead, Entry) { \
    EFI_LIST_ENTRY  *_ListHead, *_Flink; \
    _ListHead         = (ListHead); \
    _Flink            = _ListHead->Flink; \
    (Entry)->Flink    = _Flink; \
    (Entry)->Blink    = _ListHead; \
    _Flink->Blink     = (Entry); \
    _ListHead->Flink  = (Entry); \
  }

#define SwapListEntries(Entry1, Entry2) { \
    EFI_LIST_ENTRY  *Entry1Flink, *Entry1Blink; \
    EFI_LIST_ENTRY  *Entry2Flink, *Entry2Blink; \
    Entry2Flink         = (Entry2)->Flink; \
    Entry2Blink         = (Entry2)->Blink; \
    Entry1Flink         = (Entry1)->Flink; \
    Entry1Blink         = (Entry1)->Blink; \
    Entry2Blink->Flink  = Entry2Flink; \
    Entry2Flink->Blink  = Entry2Blink; \
    (Entry2)->Flink     = Entry1; \
    (Entry2)->Blink     = Entry1Blink; \
    Entry1Blink->Flink  = (Entry2); \
    (Entry1)->Blink     = (Entry2); \
  }

//
//  EFI_FIELD_OFFSET - returns the byte offset to a field within a structure
//
#define EFI_FIELD_OFFSET(TYPE, Field) ((UINTN)(&(((TYPE *) 0)->Field)))

//
// A lock structure
//
#endif
