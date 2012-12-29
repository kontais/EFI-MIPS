/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                        
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SecMain.c

Abstract:
  Linux emulator of SEC phase. It's really a Linux32 application, but this is
  Ok since all the other modules for Linux32 are NOT Linux32 applications.

  This program processes Linux environment variables and figures out
  what the memory layout will be, how may FD's will be loaded and also
  what the boot mode is. 

  The SEC registers a set of services with the SEC core. gPrivateDispatchTable
  is a list of PPI's produced by the SEC that are availble for usage in PEI.

  This code produces 128 K of temporary memory for the PEI stack by opening a
  Linux file and mapping it directly to memory addresses.

  The system.cmd script is used to set Linux environment variables that drive
  the configuration opitons of the SEC.

--*/

#include "Tiano.h"
#include "SecMain.h"

//
// Globals
//
EFI_PEI_ELF_LOADER_PROTOCOL               *gPeiEfiPeiElfLoader;
EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *gPeiEfiPeiFlushInstructionCache;
EFI_PEI_TRANSFER_CONTROL_PROTOCOL         *gPeiEfiPeiTransferControl;

//
// The SEC constructs a table of PPI's to pass up to subsequent phases.
//  This is done via the gPrivateDispatchTable.
//
#if 0
EFI_LINUX_LOAD_AS_DLL_PPI                    mSecLinuxLoadAsDllPpi = {
  SecLinuxElfLoaderLoadAsDll,
  SecLinuxElfLoaderFreeLibrary
};
#endif

LINUX_PEI_LOAD_FILE_PPI                   mSecLinuxLoadFilePpi   = { SecLinuxPeiLoadFile };
PEI_LINUX_AUTOSCAN_PPI                    mSecLinuxAutoScanPpi   = { SecLinuxPeiAutoScan };
PEI_LINUX_THUNK_PPI                       mSecLinuxThunkPpi      = { SecLinuxThunkAddress };
PEI_STATUS_CODE_PPI                       mSecStatusCodePpi      = { SecPeiReportStatusCode };
LINUX_FWH_PPI                             mSecFwhInformationPpi  = { SecLinuxFdAddress };
EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  mSecLinuxFlushCachePpi = { SecLinuxFlushCache };

EFI_PEI_PPI_DESCRIPTOR  gPrivateDispatchTable[] = {
#if 0
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gEfiLinuxLoadAsDllPpiGuid,
    &mSecLinuxLoadAsDllPpi
  },
#endif
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gLinuxPeiLoadFileGuid,
    &mSecLinuxLoadFilePpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gPeiLinuxAutoScanPpiGuid,
    &mSecLinuxAutoScanPpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gPeiLinuxThunkPpiGuid,
    &mSecLinuxThunkPpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gPeiStatusCodePpiGuid,
    &mSecStatusCodePpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gEfiPeiFlushInstructionCacheGuid,
    &mSecLinuxFlushCachePpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gLinuxFwhPpiGuid,
    &mSecFwhInformationPpi
  }
};

//
// Default information about where the FD is located.
//  This array gets filled in with information from EFI_FIRMWARE_VOLUMES
//  EFI_FIRMWARE_VOLUMES is a Linux environment variable set by system.cmd.
//  The number of array elements is allocated base on parsing
//  EFI_FIRMWARE_VOLUMES and the memory is never freed.
//
UINTN                                     gFdInfoCount = 0;
LINUX_FD_INFO                                *gFdInfo;

//
// Array that supports seperate memory rantes.
//  The memory ranges are set in system.cmd via the EFI_MEMORY_SIZE variable.
//  The number of array elements is allocated base on parsing
//  EFI_MEMORY_SIZE and the memory is never freed.
//
UINTN                                     gSystemMemoryCount = 0;
LINUX_SYSTEM_MEMORY                       *gSystemMemory;

static UINT32 EntryAddress;

int
main (
  IN  int   Argc,
  IN  char  **Argv,
  IN  char  **Envp
  )
/*++

Routine Description:
  Main entry point to SEC for Linux. This is a Linux program

Arguments:
  Argc - Number of command line arguments
  Argv - Array of command line argument strings
  Envp - Array of environmemt variable strings

Returns:
  0 - Normal exit
  1 - Abnormal exit

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  InitialStackMemory;
  UINT64                InitialStackMemorySize;
  EFI_BOOT_MODE         BootMode;
  UINTN                 Index;
  UINTN                 Index1;
  UINTN                 Index2;
  UINTN                 PeiIndex;
  CHAR8                 *MemorySizeStr;
  CHAR8                 *FirmwareVolumesStr;
  CHAR8                 *BootModeStr;
  CHAR8                 *FileName;
  BOOLEAN               Done;
  VOID                  *PeiCoreFile;
  UINTN                 *StackPointer;

  printf ("\nEDK SEC Main LINUX Emulation Environment from www.TianoCore.org\n");

  //
  // Set defaults in case we can not find an environment variable
  //
  MemorySizeStr       = "64";                  // 64MB
  FirmwareVolumesStr  = "FV/FvRecovery.fd";
  BootModeStr         = "0";

  //
  // Allocate space for gSystemMemory Array
  //
  gSystemMemoryCount  = CountSeperatorsInString (MemorySizeStr, '!') + 1;
  gSystemMemory       = calloc (gSystemMemoryCount, sizeof (LINUX_SYSTEM_MEMORY));
  if (gSystemMemory == NULL) {
    printf ("ERROR : Can not allocate memory for system.  Exiting.\n");
    exit (1);
  }
  //
  // Allocate space for gSystemMemory Array
  //
  gFdInfoCount  = CountSeperatorsInString (FirmwareVolumesStr, '!') + 1;
  gFdInfo       = calloc (gFdInfoCount, sizeof (LINUX_FD_INFO));
  if (gFdInfo == NULL) {
    printf ("ERROR : Can not allocate memory for fd info.  Exiting.\n");
    exit (1);
  }
  //
  // Setup Boot Mode. If BootModeStr == "" then BootMode = 0 (BOOT_WITH_FULL_CONFIGURATION)
  //
  BootMode = atoi (BootModeStr);
  printf ("  BootMode 0x%02x\n", BootMode);

  //
  // Open up a 128K file to emulate temp memory for PEI.
  //  on a real platform this would be SRAM, or using the cache as RAM.
  //  Set InitialStackMemory to zero so LinuxOpenFile will allocate a new mapping
  //
  InitialStackMemorySize  = STACK_SIZE;
  InitialStackMemory = (UINTN)MapMemory(0, 
          (UINT32) InitialStackMemorySize,
          PROT_READ | PROT_WRITE,
          MAP_ANONYMOUS | MAP_PRIVATE);
  if (InitialStackMemory == 0) {
    printf ("ERROR : Can not open SecStack Exiting\n");
    exit (1);
  }

  printf ("  SEC passing in %u KB of temp RAM at 0x%08x to PEI\n",
    (unsigned int)(InitialStackMemorySize / 1024),
    (unsigned long)InitialStackMemory);
    
  for (StackPointer = (UINTN*) (UINTN) InitialStackMemory;
     StackPointer < (UINTN*)(UINTN)((UINTN) InitialStackMemory + (UINT64) InitialStackMemorySize);
     StackPointer ++) {
    *StackPointer = 0x5AA55AA5;
  }

  //
  // Open All the firmware volumes and remember the info in the gFdInfo global
  //
  FileName = (CHAR8 *)malloc (strlen (FirmwareVolumesStr) + 1);
  if (FileName == NULL) {
    printf ("ERROR : Can not allocate memory for firmware volume string\n");
    exit (1);
  }

  Index2 = 0;
  for (Done = FALSE, Index = 0, PeiIndex = 0, PeiCoreFile = NULL;
       FirmwareVolumesStr[Index2] != 0;
       Index++) {
    for (Index1 = 0; (FirmwareVolumesStr[Index2] != '!') && (FirmwareVolumesStr[Index2] != 0); Index2++)
      FileName[Index1++] = FirmwareVolumesStr[Index2];
    if (FirmwareVolumesStr[Index2] == '!')
      Index2++;
    FileName[Index1]  = '\0';

    //
    // Open the FD and remmeber where it got mapped into our processes address space
    //
    Status = MapFile (
		      FileName,
		      &gFdInfo[Index].Address,
		      &gFdInfo[Index].Size
		      );
    if (EFI_ERROR (Status)) {
      printf ("ERROR : Can not open Firmware Device File %s (%x).  Exiting.\n", FileName, (unsigned int)Status);
      exit (1);
    }

    printf ("  FD loaded from %s at 0x%08x",
	    FileName, (unsigned long)gFdInfo[Index].Address);

    if (PeiCoreFile == NULL) {
      //
      // Assume the beginning of the FD is an FV and look for the PEI Core.
      // Load the first one we find.
      //
      Status = SecFfsFindPeiCore ((EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) gFdInfo[Index].Address, &PeiCoreFile);
      if (!EFI_ERROR (Status)) {
        PeiIndex = Index;
        printf (" contains SEC Core");
      }
    }

    printf ("\n");
  }
  //
  // Calculate memory regions and store the information in the gSystemMemory
  //  global for later use. The autosizing code will use this data to
  //  map this memory into the SEC process memory space.
  //
  Index1 = 0;
  Index = 0;
  while (1) {
    UINTN val = 0;
    //
    // Save the size of the memory.
    //
    while (MemorySizeStr[Index1] >= '0' && MemorySizeStr[Index1] <= '9') {
      val = val * 10 + MemorySizeStr[Index1] - '0';
      Index1++;
    }
    gSystemMemory[Index++].Size = val * 0x100000;
    if (MemorySizeStr[Index1] == 0)
      break;
    Index1++;
  }

  printf ("\n");

  //
  // Hand off to PEI Core
  //
  SecLoadFromCore ((UINTN) InitialStackMemory, (UINTN) InitialStackMemorySize, (UINTN) gFdInfo[0].Address, PeiCoreFile);

  //
  // If we get here, then the PEI Core returned. This is an error as PEI should
  //  always hand off to DXE.
  //
  printf ("ERROR : PEI Core returned\n");
  exit (1);
}

EFI_PHYSICAL_ADDRESS *
MapMemory (
  INTN fd,
  UINT64 length,
  INTN   prot,
  INTN   flags)
{
  STATIC UINTN base  = 0x40000000;
  CONST UINTN  align = (1 << 24);
  VOID         *res  = NULL;
  BOOLEAN      isAligned = 0;

  //
  // Try to get an aligned block somewhere in the address space of this
  // process.
  //
  while((!isAligned) && (base != 0)) {
    res = mmap ((void *)base, length, prot, flags, fd, 0);
    if (res == MAP_FAILED) {
      return NULL;
    }
    if ((((UINTN)res) & ~(align-1)) == (UINTN)res) {
      isAligned=1;
    }
    else {
      munmap(res, length);
      base += align;
    }
  }
  return res;
}

EFI_STATUS
MapFile (
  IN  CHAR8                     *FileName,
  IN OUT  EFI_PHYSICAL_ADDRESS  *BaseAddress,
  OUT UINT64                    *Length
  )
/*++

Routine Description:
  Opens and memory maps a file using Linux services. If BaseAddress is non zero
  the process will try and allocate the memory starting at BaseAddress.

Arguments:
  FileName            - The name of the file to open and map
  MapSize             - The amount of the file to map in bytes
  CreationDisposition - The flags to pass to CreateFile().  Use to create new files for
                        memory emulation, and exiting files for firmware volume emulation
  BaseAddress         - The base address of the mapped file in the user address space.
                         If passed in as NULL the a new memory region is used.
                         If passed in as non NULL the request memory region is used for
                          the mapping of the file into the process space.
  Length              - The size of the mapped region in bytes

Returns:
  EFI_SUCCESS      - The file was opened and mapped.
  EFI_NOT_FOUND    - FileName was not found in the current directory
  EFI_DEVICE_ERROR - An error occured attempting to map the opened file

--*/
{
  int fd;
  VOID    *res;
  UINTN   FileSize;

  fd = open (FileName, O_RDONLY);
  if (fd < 0)
    return EFI_NOT_FOUND;
  FileSize = lseek (fd, 0, SEEK_END);

#if 0
  if (IsMain)
    {
      /* Read entry address.  */
      lseek (fd, FileSize - 0x20, SEEK_SET);
      if (read (fd, &EntryAddress, 4) != 4)
	{
	  close (fd);
	  return EFI_DEVICE_ERROR;
	}      
    }
#endif

  res = MapMemory(fd, FileSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE);
  
  close (fd);

  if (res == MAP_FAILED)
    return EFI_DEVICE_ERROR;

  *Length = (UINT64) FileSize;
  *BaseAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) res;

  return EFI_SUCCESS;
}

#define BYTES_PER_RECORD  512

EFI_STATUS
EFIAPI
SecPeiReportStatusCode (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_STATUS_CODE_TYPE       CodeType,
  IN EFI_STATUS_CODE_VALUE      Value,
  IN UINT32                     Instance,
  IN EFI_GUID                  *CallerId,
  IN EFI_STATUS_CODE_DATA      *Data OPTIONAL
  )
/*++

Routine Description:

  This routine produces the ReportStatusCode PEI service. It's passed 
  up to the PEI Core via a PPI. T

  This code currently uses the LINUX clib printf. This does not work the same way 
  as the EFI Print (), as %t, %g, %s as Unicode are not supported.

Arguments:
  (see EFI_PEI_REPORT_STATUS_CODE)

Returns:
  EFI_SUCCESS - Always return success

--*/
// TODO:    PeiServices - add argument and description to function comment
// TODO:    CodeType - add argument and description to function comment
// TODO:    Value - add argument and description to function comment
// TODO:    Instance - add argument and description to function comment
// TODO:    CallerId - add argument and description to function comment
// TODO:    Data - add argument and description to function comment
{
  CHAR8           *Format;
  VA_LIST         Marker;
  CHAR8           PrintBuffer[BYTES_PER_RECORD * 2];
  CHAR8           *Filename;
  CHAR8           *Description;
  UINT32          LineNumber;
  UINT32          ErrorLevel;


  if (Data == NULL) {
  } else if (ReportStatusCodeExtractAssertInfo (CodeType, Value, Data, &Filename, &Description, &LineNumber)) {
    //
    // Processes ASSERT ()
    //
    printf ("ASSERT %s(%d): %s\n", Filename, (int)LineNumber, Description);

  } else if (ReportStatusCodeExtractDebugInfo (Data, &ErrorLevel, &Marker, &Format)) {
    //
    // Process DEBUG () macro 
    //
    AvSPrint (PrintBuffer, BYTES_PER_RECORD, Format, Marker);
    printf ("%s", PrintBuffer);
  }

  return EFI_SUCCESS;
}


VOID
SecLoadFromCore (
  IN  UINTN   LargestRegion,
  IN  UINTN   LargestRegionSize,
  IN  UINTN   BootFirmwareVolumeBase,
  IN  VOID    *PeiCorePe32File
  )
/*++

Routine Description:
  This is the service to load the PEI Core from the Firmware Volume

Arguments:
  LargestRegion           - Memory to use for PEI.
  LargestRegionSize       - Size of Memory to use for PEI
  BootFirmwareVolumeBase  - Start of the Boot FV
  PeiCorePe32File         - PEI Core PE32 

Returns:
  Success means control is transfered and thus we should never return

--*/
{
  EFI_STATUS                  Status;
  UINT64                      PeiCoreSize;
  EFI_PHYSICAL_ADDRESS        PeiImageAddress;
  EFI_PHYSICAL_ADDRESS        PeiCoreEntryPoint;
  EFI_PHYSICAL_ADDRESS        TopOfMemory;
  VOID                        *TopOfStack;
  EFI_PEI_STARTUP_DESCRIPTOR  PeiStartup;

  //
  // Install the PEI Protocols that are shared between PEI and DXE
  //
  //InstallEfiPeiFlushInstructionCache (&gPeiEfiPeiFlushInstructionCache);
  gPeiEfiPeiFlushInstructionCache = &mSecLinuxFlushCachePpi;
  InstallEfiPeiTransferControl (&gPeiEfiPeiTransferControl);

  gPeiEfiPeiElfLoader = NULL;
  InstallEfiPeiElfLoader (NULL, &gPeiEfiPeiElfLoader, NULL);

  //
  // Compute Top Of Memory for Stack and PEI Core Allocations
  //
  TopOfMemory = LargestRegion + ((LargestRegionSize) & (~15));

  //
  // Allocate 128KB for the Stack
  //
  TopOfStack  = (VOID *) (UINTN) (TopOfMemory - sizeof (EFI_PEI_STARTUP_DESCRIPTOR));
  TopOfMemory = TopOfMemory - EFI_STACK_SIZE;
  //printf("PeiStartup Size=0x%x\n",sizeof (EFI_PEI_STARTUP_DESCRIPTOR));

  //
  // Bind this information into the SEC hand-off state
  //
  PeiStartup.DispatchTable      = (EFI_PEI_PPI_DESCRIPTOR *) &gPrivateDispatchTable;
  PeiStartup.SizeOfCacheAsRam   = EFI_STACK_SIZE;
  PeiStartup.BootFirmwareVolume = BootFirmwareVolumeBase;

  CopyMem (TopOfStack, &PeiStartup, sizeof (EFI_PEI_STARTUP_DESCRIPTOR));

  //
  // Load the PEI Core from a Firmware Volume
  //
  Status = SecLinuxPeiLoadFile (
            PeiCorePe32File,
            &PeiImageAddress,
            &PeiCoreSize,
            &PeiCoreEntryPoint
            );
  if (EFI_ERROR (Status)) {
    return ;
  }

  //printf(" Addr = 0x%Lx\n",PeiImageAddress);
  //printf(" Codr = 0x%x\n",*(UINT32*)PeiImageAddress);
  //printf(" Sizr = 0x%Lx\n",PeiCoreSize);
  //printf(" Entr = 0x%Lx\n",PeiCoreEntryPoint);

  //
  // Transfer control to the PEI Core
  //
  SecSwitchStacks (
    (VOID *) (UINTN) PeiCoreEntryPoint,
    (VOID *) (UINTN) TopOfStack,
    (VOID *) (UINTN) TopOfStack,
    (VOID *) (UINTN) NULL
    );

  //
  // If we get here, then the PEI Core returned.  This is an error
  //
  return ;
}

EFI_STATUS
EFIAPI
SecLinuxPeiAutoScan (
  IN  UINTN                 Index,
  OUT EFI_PHYSICAL_ADDRESS  *MemoryBase,
  OUT UINT64                *MemorySize
  )
/*++

Routine Description:
  This service is called from Index == 0 until it returns EFI_UNSUPPORTED.
  It allows discontiguous memory regions to be supported by the emulator.
  It uses gSystemMemory[] and gSystemMemoryCount that were created by
  parsing the Linux environment variable EFI_MEMORY_SIZE.
  The size comes from the varaible and the address comes from the call to
  LinuxOpenFile. 

Arguments:
  Index      - Which memory region to use
  MemoryBase - Return Base address of memory region
  MemorySize - Return size in bytes of the memory region

Returns:
  EFI_SUCCESS - If memory region was mapped
  EFI_UNSUPPORTED - If Index is not supported

--*/
{
  void *res;

  if (Index >= gSystemMemoryCount) {
    return EFI_UNSUPPORTED;
  }

  *MemoryBase = 0;
  res = MapMemory(0, gSystemMemory[Index].Size,
		  PROT_READ | PROT_WRITE | PROT_EXEC,
		  MAP_PRIVATE | MAP_ANONYMOUS);
  if (res == MAP_FAILED)
    return EFI_DEVICE_ERROR;
  *MemorySize = gSystemMemory[Index].Size;
  *MemoryBase = (UINTN)res;
  gSystemMemory[Index].Memory = *MemoryBase;
  printf("Base=0x%x\n",(UINTN) gSystemMemory[Index].Memory);
  printf("Size=0x%x\n",(UINTN) gSystemMemory[Index].Size);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SecLinuxThunkAddress (
  IN OUT UINT64                *InterfaceSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *InterfaceBase
  )
/*++

Routine Description:
  Since the SEC is the only Linux program in stack it must export
  an interface to do Win API calls. That's what the LinuxThunk address
  is for. gLinux is initailized in LinuxThunk.c.

Arguments:
  InterfaceSize - sizeof (EFI_LINUX_THUNK_PROTOCOL);
  InterfaceBase - Address of the gLinux global

Returns:
  EFI_SUCCESS - Data returned

--*/
{
  *InterfaceSize  = sizeof (EFI_LINUX_THUNK_PROTOCOL);
  *InterfaceBase  = (EFI_PHYSICAL_ADDRESS) (UINTN) gLinux;
  printf("gLinux=0x%x\n",gLinux);
  printf("gLinux->Getevn=0x%x\n",gLinux->GetEnvironmentVariable);


  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SecLinuxPeiLoadFile (
  IN  VOID                    *Pe32Data,
  IN  EFI_PHYSICAL_ADDRESS    *ImageAddress,
  IN  UINT64                  *ImageSize,
  IN  EFI_PHYSICAL_ADDRESS    *EntryPoint
  )
/*++

Routine Description:
  Loads and relocates a PE/COFF image into memory.

Arguments:
  Pe32Data         - The base address of the PE/COFF file that is to be loaded and relocated
  ImageAddress     - The base address of the relocated PE/COFF image
  ImageSize        - The size of the relocated PE/COFF image
  EntryPoint       - The entry point of the relocated PE/COFF image

Returns:
  EFI_SUCCESS   - The file was loaded and relocated
  EFI_OUT_OF_RESOURCES - There was not enough memory to load and relocate the PE/COFF file

--*/
{
  EFI_STATUS                            Status;
  EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  ImageContext;
  void  * pLoadAddr;

  ZeroMem (&ImageContext, sizeof (ImageContext));
  ImageContext.Handle     = Pe32Data;

  ImageContext.ImageRead  = (EFI_PEI_ELF_LOADER_READ_FILE) SecImageRead;

  Status                  = gPeiEfiPeiElfLoader->GetImageInfo (gPeiEfiPeiElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Allocate space for Relocatable ELF image.
  //
  if (ImageContext.IsRelocatable) {
  ImageContext.ImageAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) malloc ((UINTN) (ImageContext.ImageSize));
    pLoadAddr = (void *) ImageContext.ImageAddress;
    if (ImageContext.ImageAddress == 0) {
      printf("Image Load Addr = 0x%x\n",(UINTN) pLoadAddr);
      printf("Image Size      = 0x%Lx\n",ImageContext.ImageSize);
      return EFI_OUT_OF_RESOURCES;
    }
  }

  //
  // 注意: ImageContext.ImageAddress在LoadImage后可能会根据区对齐而改变
  //
  Status = gPeiEfiPeiElfLoader->LoadImage (gPeiEfiPeiElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  
  Status = gPeiEfiPeiElfLoader->RelocateImage (gPeiEfiPeiElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    printf("Image Actual Addr = 0x%Lx\n", ImageContext.ImageAddress);
    printf("Image Actual Size = 0x%Lx\n",ImageContext.ImageSize);
    printf("ERROR: Relocate Status = 0x%x\n",Status);
    return Status;
  }

  Status = gPeiEfiPeiFlushInstructionCache->Flush (
                                              gPeiEfiPeiFlushInstructionCache,
                                              ImageContext.ImageAddress,
                                              ImageContext.ImageSize
                                              );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //printf("Image Actual Addr = 0x%Lx\n",ImageContext.ImageAddress);
  //printf("Image Actual Size = 0x%Lx\n",ImageContext.ImageSize);
  //printf("Image Actual Ent  = 0x%Lx\n",ImageContext.EntryPoint);
  *ImageAddress = ImageContext.ImageAddress;
  *ImageSize    = ImageContext.ImageSize;
  *EntryPoint   = ImageContext.EntryPoint;

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SecLinuxFdAddress (
  IN     UINTN                 Index,
  IN OUT EFI_PHYSICAL_ADDRESS  *FdBase,
  IN OUT UINT64                *FdSize
  )
/*++

Routine Description:
  Return the FD Size and base address. Since the FD is loaded from a 
  file into Linux memory only the SEC will know it's address.

Arguments:
  Index  - Which FD, starts at zero.
  FdSize - Size of the FD in bytes
  FdBase - Start address of the FD. Assume it points to an FV Header

Returns:
  EFI_SUCCESS     - Return the Base address and size of the FV
  EFI_UNSUPPORTED - Index does nto map to an FD in the system

--*/
{
  if (Index >= gFdInfoCount) {
    return EFI_UNSUPPORTED;
  }

  *FdBase = gFdInfo[Index].Address;
  *FdSize = gFdInfo[Index].Size;

  if (*FdBase == 0 && *FdSize == 0) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SecLinuxFlushCache (
  IN EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL *This,
  IN EFI_PHYSICAL_ADDRESS                     Start,
  IN UINT64                                   Length
  )

/*++

Routine Description:
  Linux Flush cache

Arguments:
  Start  -
  Length -

Returns:
  EFI_SUCCESS

--*/
{
  return _flush_cache((UINT8 *) Start, (UINTN) Length, BCACHE);
}

EFI_STATUS
EFIAPI
SecImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:
  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:
  FileHandle - The handle to the PE/COFF file
  FileOffset - The offset, in bytes, into the file to read
  ReadSize   - The number of bytes to read from the file starting at FileOffset
  Buffer     - A pointer to the buffer to read the data into.

Returns:
  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/
{
  CHAR8 *Destination8;
  CHAR8 *Source8;
  UINTN Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}

UINTN
CountSeperatorsInString (
  IN  CHAR8   *String,
  IN  CHAR8   Seperator
  )
/*++

Routine Description:
  Count the number of seperators in String

Arguments:
  String    - String to process
  Seperator - Item to count

Returns:
  Number of Seperator in String

--*/
{
  UINTN Count;

  for (Count = 0; *String != '\0'; String++) {
    if (*String == Seperator) {
      Count++;
    }
  }

  return Count;
}
