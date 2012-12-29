/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  fileio.h

Abstract:

  Information about the File I/O function

Revision History

--*/
#ifndef _FILE_I_O_H
#define _FILE_I_O_H

#define SIMPLE_READ_SIGNATURE EFI_SIGNATURE_32 ('s', 'r', 'd', 'r')
typedef struct {
  UINTN           Signature;
  BOOLEAN         FreeBuffer;
  VOID            *Source;
  UINTN           SourceSize;
  EFI_FILE_HANDLE FileHandle;
} SIMPLE_READ_HANDLE;

typedef VOID  *SIMPLE_READ_FILE;

EFI_FILE_HANDLE
LibOpenRoot (
  IN EFI_HANDLE                     DeviceHandle
  );

EFI_DEVICE_PATH_PROTOCOL              *
DuplicateDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  );

EFI_STATUS
LibSplitFsAndPath (
  IN     CHAR16                       *AbPath,
  IN OUT CHAR16                       **Fs,
  IN OUT CHAR16                       **Path
  );

EFI_FILE_INFO                   *
LibGetFileInfo (
  IN EFI_FILE_HANDLE      FileHand
  );

EFI_STATUS
LibSetFileInfo (
  IN EFI_FILE_HANDLE  FileHand,
  IN UINTN            BufferSize,
  IN VOID             *Buffer
  );

EFI_STATUS
LibOpenFile (
  IN OUT EFI_DEVICE_PATH_PROTOCOL       **FilePath,
  OUT EFI_HANDLE                        *DeviceHandle,
  OUT EFI_FILE_HANDLE                   *Handle,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes

  );

EFI_STATUS
LibOpenFileByName (
  IN  CHAR16                            *FileName,
  OUT EFI_FILE_HANDLE                   *FileHandle,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes
  );

EFI_STATUS
LibCreateDirectory (
  IN CHAR16                            *DirName,
  OUT EFI_FILE_HANDLE                  *FileHandle
  );

EFI_STATUS
LibReadFile (
  IN EFI_FILE_HANDLE      FileHandle,
  IN OUT UINTN            *ReadSize,
  OUT VOID                *Buffer
  );

EFI_STATUS
LibWriteFile (
  IN EFI_FILE_HANDLE  FileHandle,
  IN OUT UINTN        *BufferSize,
  OUT VOID            *Buffer
  );

EFI_STATUS
LibCloseFile (
  IN EFI_FILE_HANDLE     FileHandle
  );

EFI_STATUS
LibDeleteFile (
  IN EFI_FILE_HANDLE     FileHandle
  );

EFI_STATUS
LibSetPosition (
  IN EFI_FILE_HANDLE      FileHand,
  OUT UINT64              Position
  );

EFI_STATUS
LibGetPosition (
  IN EFI_FILE_HANDLE     FileHand,
  OUT UINT64             *Position
  );

EFI_STATUS
LibFlushFile (
  IN EFI_FILE_HANDLE    FileHand
  );

EFI_STATUS
LibFindFirstFile (
  IN     EFI_FILE_HANDLE                    DirHandle,
  OUT EFI_FILE_INFO                         *Buffer
  );

EFI_STATUS
LibFindNextFile (
  IN     EFI_FILE_HANDLE                    DirHandle,
  OUT EFI_FILE_INFO                         *Buffer,
  OUT BOOLEAN                               *NoFile
  );

EFI_STATUS
LibGetFileSize (
  IN     EFI_FILE_HANDLE                    DirHandle,
  OUT UINT64                                *Size
  );

EFI_DEVICE_PATH_PROTOCOL        *
LibFileDevicePath (
  IN EFI_HANDLE                 Device  OPTIONAL,
  IN CHAR16                     *FileName
  );

EFI_STATUS
LibIsSubPath (
  IN CHAR16                       *Parent,
  IN CHAR16                       *Child,
  OUT BOOLEAN                     *Result
  );

CHAR16                    *
LibFormAbsolutePath (
  IN CHAR16                       *FilePath,
  IN CHAR16                       *Ref OPTIONAL
  );

EFI_STATUS
LibCompareFile (
  IN CHAR16                      *DstFile,
  IN CHAR16                      *SrcFile,
  IN OUT BOOLEAN                 *IsSame
  );

EFI_FILE_HANDLE
LibOpenFilePath (
  IN EFI_DEVICE_PATH_PROTOCOL     *FilePath,
  IN UINT64                       FileMode
  );

BOOLEAN
CompareFsDevice (
  IN  CHAR16     *FsName1,
  IN  CHAR16     *FsName2
  );
  
EFI_STATUS
OpenSimpleReadFile (
  IN BOOLEAN                        BootPolicy,
  IN VOID                           *SourceBuffer OPTIONAL,
  IN UINTN                          SourceSize,
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **FilePath,
  OUT EFI_HANDLE                    *DeviceHandle,
  OUT SIMPLE_READ_FILE              *SimpleReadHandle
  );

EFI_STATUS
ReadSimpleReadFile (
  IN SIMPLE_READ_FILE               SimpleReadHandle,
  IN UINTN                          Offset,
  IN OUT UINTN                      *ReadSize,
  OUT VOID                          *Buffer
  );

VOID
CloseSimpleReadFile (
  IN SIMPLE_READ_FILE               SimpleReadHandle
  );

#endif