/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Ext2.h

Abstract:


--*/

#ifndef _EXT2_H_
#define _EXT2_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "LinkedList.h"

#include "Ext2Disk.h"
#include "Inode.h"

//#define EXT2_DEBUG

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (BlockIo)
#include EFI_PROTOCOL_DEFINITION (DiskIo)

//
// Driver Consumed Guids
//
#include EFI_GUID_DEFINITION (Gpt)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleFileSystem)
#include EFI_PROTOCOL_DEFINITION (FileInfo)
#include EFI_PROTOCOL_DEFINITION (FileSystemInfo)
#include EFI_PROTOCOL_DEFINITION (FileSystemVolumeLabelInfo)



EFI_FORWARD_DECLARATION (EXT2_VOLUME);
EFI_FORWARD_DECLARATION (EXT2_OFILE);
EFI_FORWARD_DECLARATION (EXT2_IFILE);


#define INVALID_BLOCK_NO (0xffffffff)
//
// Path name separator is back slash
//
#define PATH_NAME_SEPARATOR     L'\\'

//
// Used in Ext2DiskIo
//
typedef enum {
  READ_DATA     = 0,  // read data cache
  WRITE_DATA    = 1   // write data cache
} IO_MODE;

//
// private data structures
//
#define CompareGuid EfiCompareGuid
#define AllocateZeroPool EfiLibAllocateZeroPool

#define EXT2_VOLUME_SIGNATURE  EFI_SIGNATURE_32 ('e', 'x', '2', 'V')
#define EXT2_IFILE_SIGNATURE   EFI_SIGNATURE_32 ('e', 'x', '2', 'I')
#define EXT2_OFILE_SIGNATURE   EFI_SIGNATURE_32 ('e', 'x', '2', 'O')

#define ASSERT_VOLUME_LOCKED(a)      ASSERT_LOCKED (&Ext2FsLock)

typedef struct _BLOCK_PTR {
  VOID    *Buffer;
  UINT32  BufferTag;
} BLOCK_PTR;

struct _EXT2_IFILE {
  UINTN               Signature;
  EFI_FILE            Handle;
  UINT64              Position;
  BOOLEAN             ReadOnly;
  EXT2_OFILE          *OFile;
  CHAR16              Name[EXT2_NAME_LEN + 1];
  EFI_LIST_ENTRY      Link;
};

//
// EXT2_OFILE - Each opened inode
//
struct _EXT2_OFILE {
  UINTN               Signature;
  EXT2_VOLUME         *Volume;
  //
  // A permanant error code to return to all accesses to
  // this opened file
  //
  EFI_STATUS          Error;
  //
  // A list of the IFILE instances for this OFile
  //
  EFI_LIST_ENTRY      Opens;

  UINT32              InodeNum;
  EXT2_INODE          Inode;
  BLOCK_PTR           BlockPtr[3];
  
  UINT64              Size;
  UINT16              Mode;
  CHAR16              OpenName[EXT2_NAME_LEN + 1];

  //
  // Set by an OFile SetPosition
  //
  UINTN               Position; // within file
  UINT64              PosDisk;  // on the disk
  UINTN               PosRem;   // remaining in this disk run
  //
  // The opened parent, full path length and currently opened child files
  //
  EXT2_OFILE          *Parent;
  EFI_LIST_ENTRY      ChildHead;
  EFI_LIST_ENTRY      ChildLink;

  //
  // Link in Volume's reference list
  //
  EFI_LIST_ENTRY      CheckLink;

  //
  // Link in Volume's open file list
  //
  EFI_LIST_ENTRY      OpenedLink;
};

struct _EXT2_VOLUME {
  UINTN                           Signature;

  EFI_HANDLE                      Handle;
  BOOLEAN                         Valid;
  BOOLEAN                         DiskError;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL VolumeInterface;

  //
  // If opened, the parent handle and BlockIo interface
  //
  EFI_BLOCK_IO_PROTOCOL           *BlockIo;
  EFI_DISK_IO_PROTOCOL            *DiskIo;
  UINT32                          MediaId;
  BOOLEAN                         ReadOnly;

  //
  // Ext2 file system info
  //
  UINT64                          VolumeSize;
  UINT32                          BlockSize;
  UINT32                          BlockCount;
  UINT32                          FreeBlockCount;
  UINT32                          InodeSize;
  UINT32                          FirstDataBlock;
  UINT32                          InodesPerGroup;
  CHAR16                          Label[16];
  
  UINT32                          IndBlockCount;
  UINT32                          DIndBlockCount;

  EXT2_OFILE                      *RootInode;

  //
  // New OFiles are added to this list so they
  // can be cleaned up if they aren't referenced.
  //
  EFI_LIST_ENTRY                  CheckRef;

  EFI_LIST_ENTRY                  OFiles;

  VOID                            *Buffer;
  UINT32                          BufferBlock;
};


#define EXT2_IFILE_FROM_FILE_HANDLE(a)   CR (a, EXT2_IFILE, Handle, EXT2_IFILE_SIGNATURE)
#define IFILE_FROM_OFILE(a)              CR (a, EXT2_IFILE, OFile, EXT2_IFILE_SIGNATURE)
#define EXT2_VOLUME_FROM_ROOT_INODE(a)   CR (a, EXT2_VOLUME, RootInode, EXT2_VOLUME_SIGNATURE)
#define EXT2_VOLUME_FROM_FILE_SYSTEM(a)  CR (a, EXT2_VOLUME, VolumeInterface, EXT2_VOLUME_SIGNATURE)
#define OFILE_FROM_CHECKLINK(a)          CR (a, EXT2_OFILE, CheckLink, EXT2_OFILE_SIGNATURE)
#define OFILE_FROM_CHILDLINK(a)          CR (a, EXT2_OFILE, ChildLink, EXT2_OFILE_SIGNATURE)
#define OFILE_FROM_OPENEDLINK(a)         CR (a, EXT2_OFILE, OpenedLink, EXT2_OFILE_SIGNATURE)





//
// Super.c
//
EFI_STATUS
EFIAPI
Ext2ReadSuperBlock (
  IN OUT EXT2_VOLUME     *Volume
  );



//
// Function Prototypes
//
EFI_STATUS
EFIAPI
Ext2Open (
  IN  EFI_FILE          *FHand,
  OUT EFI_FILE          **NewHandle,
  IN  CHAR16            *FileName,
  IN  UINT64            OpenMode,
  IN  UINT64            Attributes
  )
/*++
Routine Description:

  Implements Open() of Simple File System Protocol.

Arguments:

  FHand                 - File handle of the file serves as a starting reference point.
  NewHandle             - Handle of the file that is newly opened.
  FileName              - File name relative to FHand.
  OpenMode              - Open mode.
  Attributes            - Attributes to set if the file is created.

Returns:

  EFI_INVALID_PARAMETER - The FileName is NULL or the file string is empty.
                          The OpenMode is not supported.
                          The Attributes is not the valid attributes.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory for file string.
  EFI_SUCCESS           - Open the file successfully.
  Others                - The status of open file.

--*/
;

EFI_STATUS
EFIAPI
Ext2GetPosition (
  IN  EFI_FILE          *FHand,
  OUT UINT64            *Position
  )
/*++

Routine Description:

  Get the file's position of the file

Arguments:

  FHand                 - The handle of file.
  Position              - The file's position of the file.

Returns:

  EFI_SUCCESS           - Get the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_UNSUPPORTED       - The open file is not a file.

--*/
;

EFI_STATUS
EFIAPI
Ext2GetInfo (
  IN     EFI_FILE               *FHand,
  IN     EFI_GUID               *Type,
  IN OUT UINTN                  *BufferSize,
     OUT VOID                   *Buffer
  )
/*++

Routine Description:

  Get the some types info of the file into Buffer

Arguments:

  FHand                 - The handle of file.
  Type                  - The type of the info.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing volume info.

Returns:

  EFI_SUCCESS           - Get the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.

--*/
;

EFI_STATUS
EFIAPI
Ext2SetInfo (
  IN EFI_FILE           *FHand,
  IN EFI_GUID           *Type,
  IN UINTN              BufferSize,
  IN VOID               *Buffer
  )
/*++

Routine Description:

  Set the some types info of the file into Buffer

Arguments:

  FHand                 - The handle of file.
  Type                  - The type of the info.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing volume info.

Returns:

  EFI_SUCCESS           - Set the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.

--*/
;

EFI_STATUS
EFIAPI
Ext2Flush (
  IN EFI_FILE           *FHand
  )
/*++

Routine Description:

  Flushes all data associated with the file handle

Arguments:

  FHand                 - Handle to file to flush

Returns:

  EFI_SUCCESS           - Flushed the file successfully
  EFI_WRITE_PROTECTED   - The volume is read only
  EFI_ACCESS_DENIED     - The volume is not read only
                          but the file is read only
  Others                - Flushing of the file is failed

--*/
;

EFI_STATUS
EFIAPI
Ext2Close (
  IN EFI_FILE           *FHand
  )
/*++

Routine Description:

  Flushes & Closes the file handle.

Arguments:

  FHand                 - Handle to the file to delete.

Returns:

  EFI_SUCCESS           - Closed the file successfully.

--*/
;

EFI_STATUS
EFIAPI
Ext2Delete (
  IN EFI_FILE           *FHand
  )
/*++

Routine Description:

  Deletes the file & Closes the file handle.

Arguments:

  FHand                    - Handle to the file to delete.

Returns:

  EFI_SUCCESS              - Delete the file successfully.
  EFI_WARN_DELETE_FAILURE  - Fail to delete the file.

--*/
;

EFI_STATUS
EFIAPI
Ext2SetPosition (
  IN EFI_FILE           *FHand,
  IN UINT64             Position
  )
/*++

Routine Description:

  Set the file's position of the file

Arguments:

  FHand                 - The handle of file
  Position              - The file's position of the file

Returns:

  EFI_SUCCESS           - Set the info successfully
  EFI_DEVICE_ERROR      - Can not find the OFile for the file
  EFI_UNSUPPORTED       - Set a directory with a not-zero position

--*/
;

EFI_STATUS
EFIAPI
Ext2Read (
  IN     EFI_FILE             *FHand,
  IN OUT UINTN                *BufferSize,
     OUT VOID                 *Buffer
  )
/*++

Routine Description:

  Get the file info.

Arguments:

  FHand                 - The handle of the file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  other                 - An error occurred when operation the disk.

--*/
;

EFI_STATUS
EFIAPI
Ext2Write (
  IN     EFI_FILE               *FHand,
  IN OUT UINTN                  *BufferSize,
  IN     VOID                   *Buffer
  )
/*++

Routine Description:

  Set the file info.

Arguments:

  FHand                 - The handle of the file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing write data.

Returns:

  EFI_SUCCESS           - Set the file info successfully.
  EFI_WRITE_PROTECTED   - The disk is write protected.
  EFI_ACCESS_DENIED     - The file is read-only.
  EFI_DEVICE_ERROR      - The OFile is not valid.
  EFI_UNSUPPORTED       - The open file is not a file.
                        - The writing file size is larger than 4GB.
  other                 - An error occurred when operation the disk.

--*/
;

//
// Flush.c
//
EFI_STATUS
Ext2OFileFlush (
  IN EXT2_OFILE          *OFile
  );

BOOLEAN
Ext2CheckOFileRef (
  IN EXT2_OFILE          *OFile
  );

VOID
Ext2SetVolumeError (
  IN EXT2_OFILE          *OFile,
  IN EFI_STATUS         Status
  );

EFI_STATUS
Ext2IFileClose (
  EXT2_IFILE             *IFile
  );

EFI_STATUS
Ext2CleanupVolume (
  IN EXT2_VOLUME         *Volume,
  IN EXT2_OFILE          *OFile,
  IN EFI_STATUS         EfiStatus
  );

//
// FileSpace.c
//

EFI_STATUS
Ext2GrowEof (
  IN EXT2_OFILE         *OFile,
  IN UINT64             NewSizeInBytes
  );


EFI_STATUS
Ext2OFilePosition (
  IN EXT2_OFILE           *OFile,
  IN UINTN                Position,
  IN UINTN                PosLimit
  );

// Init.c
//
EFI_STATUS
Ext2AllocateVolume (
  IN  EFI_HANDLE                     Handle,
  IN  EFI_DISK_IO_PROTOCOL           *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL          *BlockIo
  );

EFI_STATUS
Ext2OpenDevice (
  IN OUT EXT2_VOLUME     *Volume
  );

EFI_STATUS
Ext2AbandonVolume (
  IN EXT2_VOLUME         *Volume
  );

//
// Misc.c
//

EFI_STATUS
Ext2DiskIo (
  IN EXT2_VOLUME        *Volume,
  IN IO_MODE            IoMode,
  IN UINT64             Offset,
  IN UINTN              BufferSize,
  IN OUT VOID           *Buffer
  );

VOID
Ext2FreeVolume (
  IN EXT2_VOLUME         *Volume
  );

VOID
Ext2AcquireLock (
  VOID
  );

VOID
Ext2ReleaseLock (
  VOID
  );

EFI_STATUS
Ext2AcquireLockOrFail (
  VOID
  );

VOID 
Ext2TimeToEfiTime (
  IN  UINT32        SecTime,
  OUT EFI_TIME      *ETime
  );

//
// Open.c
//
EFI_STATUS
Ext2OFileOpen (
  IN EXT2_OFILE         *OFile,
  OUT EXT2_IFILE        **NewIFile,
  IN CHAR16             *FileName,
  IN UINT64             OpenMode,
  IN UINT8              Attributes
  );

EFI_STATUS
Ext2AllocateIFile (
  IN EXT2_OFILE         *OFile,
  OUT EXT2_IFILE        **PtrIFile
  );

//
// OpenVolume.c
//
EFI_STATUS
EFIAPI
Ext2OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
  OUT EFI_FILE                        **File
  );

//
// ReadWrite.c
//
EFI_STATUS
Ext2AccessOFile (
  IN EXT2_OFILE         *OFile,
  IN IO_MODE            IoMode,
  IN UINTN              Position,
  IN UINTN              *DataBufferSize,
  IN UINT8              *UserBuffer
  );

//
// DirectoryManage.c
//
EFI_STATUS
Ext2GetNextDirEntry (
  IN  EXT2_OFILE         *OFILE,
  OUT EXT2_DIR_ENTRY     *DirEntry
  );

EFI_STATUS
Ext2GetDirEntryInfo (
  IN EXT2_VOLUME         *Volume,
  IN CHAR16              *Name,
  IN UINT32              InodeNum,
  IN OUT UINTN          *BufferSize,
  OUT VOID              *Buffer
  );


EFI_STATUS
Ext2SearchDir (
  IN  EXT2_OFILE     *OFile,
  IN  CHAR16         *FileNameString,
  OUT ino_t          *InodeNum
  );

EFI_STATUS
Ext2LocateOFile (
  IN OUT EXT2_OFILE       **PtrOFile,
  IN     CHAR16           *FileName,
  IN     UINT8            Attributes,
  OUT    CHAR16           *NewFileName
  );

//
// Inode.c
//
EFI_STATUS
EFIAPI
Ext2OpenInode (
  IN  UINT32         InodeNum,
  IN  EXT2_OFILE     *Parent OPTIONAL,
  IN  CHAR16         *OpenName OPTIONAL,
  OUT EXT2_OFILE     **NewInode
  );

EFI_STATUS
EFIAPI
Ext2CloseInode (
  IN EXT2_OFILE   *OFile
  );

EFI_STATUS
Ext2FileBlockToVolumeBlock (
  IN EXT2_OFILE   *OFile,
  IN UINT32       FileBlockIndex,
  OUT UINT32      *VolumeBlockIndex
  );

//
// FileName.c
//

CHAR16*
Ext2GetNextNameComponent (
  IN  CHAR16            *Path,
  OUT CHAR16            *Name
  );

BOOLEAN
Ext2FileNameIsValid (
  IN  CHAR16  *InputFileName,
  OUT CHAR16  *OutputFileName
  );

// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL     gExt2DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL     gExt2ComponentName;
extern EFI_LOCK                        Ext2FsLock;
extern EFI_FILE                        Ext2FileInterface;

#endif
