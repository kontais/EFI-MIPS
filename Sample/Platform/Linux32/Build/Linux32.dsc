#/*++
#
# Copyright (c) 2004 - 2007, Intel Corporation                                                         
# All rights reserved. This program and the accompanying materials                          
# are licensed and made available under the terms and conditions of the BSD License         
# which accompanies this distribution.  The full text of the license may be found at        
# http://opensource.org/licenses/bsd-license.php                                            
#                                                                                           
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
# 
#  Module Name:
#
#    Linux32.dsc
#
#  Abstract:
#
#    This is the build description file containing the platform 
#    build definitions.
#
#
#  Notes:
#    
#    The info in this file is broken down into sections. The start of a section
#    is designated by a "[" in the first column. So the [=====] separater ends
#    a section.
#    
#--*/


[=============================================================================]
#
# This section gets processed first by the utility. Define any
# macros that you may use elsewhere in this description file. This is the
# mechanism by which you can pass parameters and defines to the makefiles
# generated for each component. You can define it here, and then make an
# assignment in the [makefile.common] section. For example, if here you
# define MY_VAR = my_var_value, then you can add MY_VAR = $(MY_VAR) in
# the [makefile.common] section and it becomes MY_VAR = my_var_value in
# the output makefiles for each component.
#
[Defines]
PLATFORM                  = $(PROJECT_NAME)
FV_RECOVERY_BASE_ADDRESS  = 0xFFD80000
NV_STORAGE_BASE_ADDRESS   = 0xFFFC0000
#LANGUAGE                  = eng,fra,chn 

#
# Actual variable block size should exclude the size of the FV header.
# Since FV header may vary when different block maps are used, we
# regard the FV header as part of the variable block.
#
EFI_NV_VARIABLE_STORE_LENGTH  = 0xC000
EFI_NV_EVENT_LOG_LENGTH       = 0x2000
EFI_NV_FTW_WORKING_LENGTH     = 0x2000
EFI_NV_FTW_SPARE_LENGTH       = 0x10000

[=============================================================================]
#
# Include other common build descriptions
#
!include "$(EDK_SOURCE)/Sample/Platform/Common.dsc"
!include "$(EDK_SOURCE)/Sample/Platform/Common$(PROCESSOR).dsc"

[=============================================================================]
#
# These get emitted to the output makefile for each FV being built.
#

[=============================================================================]
[Build.Fv.FvRecovery,Build.Fv.NvStorage,Build.Fv.FvFile]
#
# Commands to build firmware volumes using the FV INF file
#
FV/$(FV_FILENAME).fv : FV/$(FV_FILENAME).inf $($(FV_FILENAME)_FILES)
	cd $(@D) &&	$(GENFVIMAGE) -I $(FV_FILENAME).inf
[=============================================================================]
#
# These control the generation of the FV files
#
[=============================================================================]
[Fv.FvRecovery.Options]
EFI_BASE_ADDRESS        = $(FV_RECOVERY_BASE_ADDRESS)
EFI_FILE_NAME           = FvRecovery.fv
EFI_NUM_BLOCKS          = 0x28
EFI_BLOCK_SIZE          = 0x10000

[Fv.FvFile.Options]
EFI_BASE_ADDRESS        = 0x00000000
EFI_FILE_NAME           = $(FV_FILENAME).fv
EFI_SYM_FILE_NAME       = $(FV_FILENAME).sym
EFI_NUM_BLOCKS          = 0x2
EFI_BLOCK_SIZE          = 0x10000


[=============================================================================]
[Fv.NvStorage.Options]
EFI_BASE_ADDRESS        = $(NV_STORAGE_BASE_ADDRESS)
EFI_FILE_NAME           = $(FV_FILENAME).fv
EFI_NUM_BLOCKS          = 0x2
EFI_BLOCK_SIZE          = 0x10000
EFI_FV_GUID             = fff12b8d-7696-4c8b-a985-2747075b4f50

[=============================================================================]
[Fv.NvStorage.Components]
EFI_NV_VARIABLE         = $(EFI_NV_VARIABLE_STORE_LENGTH)
EFI_NV_EVENT_LOG        = $(EFI_NV_EVENT_LOG_LENGTH)
EFI_NV_FTW_WORKING      = $(EFI_NV_FTW_WORKING_LENGTH)
EFI_NV_FTW_SPARE        = $(EFI_NV_FTW_SPARE_LENGTH)

[=============================================================================]
[Fv.FvRecovery.Attributes,Fv.FvFile.Attributes,Fv.NvStorage.Attributes]
EFI_READ_DISABLED_CAP   = TRUE
EFI_READ_ENABLED_CAP    = TRUE
EFI_READ_STATUS         = TRUE
EFI_WRITE_DISABLED_CAP  = TRUE
EFI_WRITE_ENABLED_CAP   = TRUE
EFI_WRITE_STATUS        = TRUE
EFI_LOCK_CAP            = TRUE
EFI_LOCK_STATUS         = TRUE
EFI_STICKY_WRITE        = FALSE
EFI_MEMORY_MAPPED       = TRUE
EFI_ERASE_POLARITY      = 1
EFI_ALIGNMENT_CAP       = TRUE
EFI_ALIGNMENT_2         = TRUE
EFI_ALIGNMENT_4         = TRUE
EFI_ALIGNMENT_8         = TRUE
EFI_ALIGNMENT_16        = TRUE
EFI_ALIGNMENT_32        = TRUE
EFI_ALIGNMENT_64        = TRUE
EFI_ALIGNMENT_128       = TRUE
EFI_ALIGNMENT_256       = TRUE
EFI_ALIGNMENT_512       = TRUE
EFI_ALIGNMENT_1K        = TRUE
EFI_ALIGNMENT_2K        = TRUE
EFI_ALIGNMENT_4K        = TRUE
EFI_ALIGNMENT_8K        = TRUE
EFI_ALIGNMENT_16K       = TRUE
EFI_ALIGNMENT_32K       = TRUE
EFI_ALIGNMENT_64K       = TRUE

[=============================================================================]
[Capsule.Capsule_A.Options]
EFI_BASE_ADDRESS        = $(FV_RECOVERY_BASE_ADDRESS)
EFI_FILE_NAME           = Capsule_A.fv
EFI_NUM_BLOCKS          = 0x28
EFI_BLOCK_SIZE          = 0x10000

[=============================================================================]
[Capsule.Capsule_A.Attributes]
EFI_READ_DISABLED_CAP   = TRUE
EFI_READ_ENABLED_CAP    = TRUE
EFI_READ_STATUS         = TRUE
EFI_WRITE_DISABLED_CAP  = TRUE
EFI_WRITE_ENABLED_CAP   = TRUE
EFI_WRITE_STATUS        = TRUE
EFI_LOCK_CAP            = TRUE
EFI_LOCK_STATUS         = TRUE
EFI_STICKY_WRITE        = FALSE
EFI_MEMORY_MAPPED       = TRUE
EFI_ERASE_POLARITY      = 1
EFI_ALIGNMENT_CAP       = TRUE
EFI_ALIGNMENT_2         = TRUE
EFI_ALIGNMENT_4         = TRUE
EFI_ALIGNMENT_8         = TRUE
EFI_ALIGNMENT_16        = TRUE
EFI_ALIGNMENT_32        = TRUE
EFI_ALIGNMENT_64        = TRUE
EFI_ALIGNMENT_128       = TRUE
EFI_ALIGNMENT_256       = TRUE
EFI_ALIGNMENT_512       = TRUE
EFI_ALIGNMENT_1K        = TRUE
EFI_ALIGNMENT_2K        = TRUE
EFI_ALIGNMENT_4K        = TRUE
EFI_ALIGNMENT_8K        = TRUE
EFI_ALIGNMENT_16K       = TRUE
EFI_ALIGNMENT_32K       = TRUE
EFI_ALIGNMENT_64K       = TRUE

[=============================================================================]
#
# These are the libraries that will be built by the master makefile
#
[=============================================================================]
[Libraries]
DEFINE EDK_PREFIX=

!include "$(EDK_SOURCE)/Sample/Platform/EdkLibAll.dsc"

#
# EdkII Glue Library
#
#!include "$(EDK_SOURCE)/Sample/Platform/EdkIIGlueLibAll.dsc"

[=============================================================================]
#
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]
Sample/Platform/Linux32/Ppi/EdkLinux32PpiLib.inf
Sample/Platform/Linux32/Protocol/EdkLinux32ProtocolLib.inf
Sample/Cpu/LinuxThunk/Sec/SecMain.inf
Sample/Library/Dxe/Linux/LinuxLib.inf
Sample/Platform/Generic/MonoStatusCode/Library/Pei/MemoryStatusCode/MemoryStatusCode.inf
Sample/Platform/Generic/RuntimeDxe/StatusCode/Lib/BsDataHubStatusCode/BsDataHubStatusCode.inf
Sample/Platform/Generic/RuntimeDxe/StatusCode/Lib/RtMemoryStatusCode/RtMemoryStatusCode.inf
Sample/Platform/Generic/RuntimeDxe/StatusCode/Lib/RtPlatformStatusCode/$(PROJECT_NAME)/RtPlatformStatusCode.inf

Sample/Universal/DxeIpl/Library/DxeIplLib.inf

Sample/Bus/Scsi/ScsiLib/Dxe/ScsiLib.inf
Sample/Universal/Network/Library/NetLib.inf
#Sample/Bus/Isa/IsaDriverLib.inf
#Sample/Universal/Console/Library/ConsoleLib/ConsoleLib.inf

[=============================================================================]
#
# These are the components that will be built by the master makefile
#
[=============================================================================]
[Components]

#
# The default package
#
DEFINE PACKAGE=Default

#
# By default components in this section belong in the recovery FV.
# This FV primarily contains PEI code.
#
DEFINE FV=FVRECOVERY


#
# SEC Core
#
# SEC core is a separate utility built by the makefile.

#
# PEI Core
#
Foundation/Core/Pei/PeiMain.inf

#
# PEIM
#
Sample/Platform/Generic/MonoStatusCode/Pei/$(PROJECT_NAME)/MonoStatusCode.inf
Sample/Platform/Linux32/Pei/BootMode/BootMode.inf
Sample/Platform/Linux32/Pei/FlashMap/FlashMap.inf
Sample/Universal/GenericMemoryTest/Pei/BaseMemoryTest.inf
Sample/Universal/Variable/Pei/Variable.inf
Sample/Platform/Linux32/Pei/Autoscan/LinuxAutoScanPei.inf
Sample/Platform/Linux32/Pei/FirmwareVolume/LinuxFwhPei.inf
Sample/Universal/DxeIpl/Pei/DxeIpl.inf
Sample/Platform/Linux32/Pei/LinuxStuff/LinuxStuff.inf

#
# DXE Core
#
Foundation/Core/Dxe/DxeMain.inf                                                  PACKAGE=DxeMain

#
# APRIORI list, this is a list of drivers run without dependencies
#
#Sample/Universal/FirmwareVolume/Apriori/Dxe/AprioriList.inf FV=FVRECOVERY

#
# Guided Section Extraction Protocol used to authenticate images
#
Sample/Universal/FirmwareVolume/GuidedSectionExtraction/Crc32SectionExtract/Dxe/Crc32SectionExtract.inf

#
# Components that produce the architectural protocols
#
Sample/Cpu/LinuxThunk/CpuIo/Dxe/CpuIo.inf
Sample/Cpu/LinuxThunk/Cpu/Dxe/Cpu.inf
#Sample/Cpu/Loongson/CpuIo/RuntimeDxe/CpuIo.inf
#Sample/Cpu/Loongson/SimpleCpu/Dxe/Cpu.inf
Sample/Chipset/LinuxThunk/Metronome/Dxe/Metronome.inf
Sample/Chipset/LinuxThunk/Timer/Dxe/Timer.inf
#Sample/Chipset/AdvancedConfiguration/AcpiMetronome/Dxe/AcpiMetronome.inf
#Sample/Chipset/AdvancedConfiguration/SmartTimer/Dxe/SmartTimer.inf
#Sample/Chipset/AdvancedConfiguration/AcpiReset/RuntimeDxe/Reset.inf
#Sample/Chipset/PcCompatible/8254Timer/Dxe/8254Timer.inf
#Sample/Chipset/PcCompatible/8259/Dxe/8259.inf
#Sample/Chipset/PcCompatible/IsaAcpi/Dxe/IsaAcpi.inf
#Sample/Chipset/PcCompatible/KbcReset/RuntimeDxe/Reset.inf
#Sample/Chipset/PcCompatible/Metronome/Dxe/Metronome.inf
#Sample/Chipset/PcCompatible/PciRootBridgeNoEnumeration/Dxe/PciRootBridgeNoEnumeration.inf
#Sample/Chipset/PcCompatible/Port92Reset/RuntimeDxe/Reset.inf
#Sample/Chipset/PcCompatible/RealTimeClock/RuntimeDxe/RealTimeClock.inf
Sample/Universal/WatchdogTimer/Dxe/WatchDogTimer.inf
Sample/Chipset/LinuxThunk/RealTimeClock/RuntimeDxe/RealTimeClock.inf
Sample/Chipset/LinuxThunk/Reset/RuntimeDxe/Reset.inf
Sample/Universal/Runtime/Dxe/Runtime.inf
Sample/Universal/MonotonicCounter/RuntimeDxe/MonotonicCounter.inf
Sample/Platform/Generic/RuntimeDxe/StatusCode/StatusCode.inf
Sample/Platform/Generic/RuntimeDxe/FvbServices/Linux32Fwh.inf
Sample/Universal/FirmwareVolume/FaultTolerantWriteLite/Dxe/FtwLite.inf
Sample/Universal/Variable/RuntimeDxe/Variable.inf
Sample/Platform/Linux32/Dxe/PlatformBds/PlatformBds.inf
Sample/Universal/Security/SecurityStub/Dxe/SecurityStub.inf
$(CAPSULE_INF)

#
# Drivers necessary to use the debugger
# These are placed here so they will load early if they are desired.
# NOTE:
#   Including these in the FV will take over use of the COM port.
#
Sample/Universal/Debugger/Debugport/Dxe/DebugPort.inf                    FV=NULL 
Sample/Cpu/DebugSupport/Dxe/DebugSupport.inf                             FV=NULL

#
# Following are the DXE drivers (alphabetical order)
#

Sample/Platform/Generic/Dxe/ConPlatform/ConPlatform.inf
Sample/Universal/Console/ConSplitter/Dxe/ConSplitter.inf
Sample/Universal/Console/GraphicsConsole/Dxe/GraphicsConsole.inf
Sample/Universal/Console/Terminal/Dxe/Terminal.inf
#Sample/Universal/Console/VgaClass/Dxe/VgaClass.inf
#Sample/Universal/Console/VgaClass/Dxe/VgaClassLite.inf
Sample/Universal/DataHub/DataHub/Dxe/DataHub.inf
Sample/Universal/DataHub/DataHubStdErr/Dxe/DataHubStdErr.inf
$(DEVPATH_INF)
Sample/Universal/Disk/DiskIo/Dxe/DiskIo.inf
Sample/Universal/Ebc/Dxe/Ebc.inf
Sample/Universal/GenericMemoryTest/Dxe/NullMemoryTest.inf
Sample/Universal/UserInterface/HiiDataBase/Dxe/HiiDatabase.inf
Sample/Platform/Generic/Logo/Logo.inf
Sample/Universal/Disk/Partition/Dxe/Partition.inf
Sample/Bus/Pci/PciBus/Dxe/PciBus.inf
#Sample/Bus/Pci/PciBusNoEnumeration/Dxe/PciBusNoEnumeration.inf
Sample/Universal/UserInterface/SetupBrowser/Dxe/SetupBrowser.inf                   
Sample/Universal/UserInterface/SetupBrowser/Dxe/DriverSample/DriverSample.inf
#Other/Maintained/Application/$(UEFI_PREFIX)Shell/Bin/Shell.inf
Other/Maintained/Application/$(UEFI_PREFIX)Shell/Shell.inf
Sample/Universal/Disk/UnicodeCollation/English/Dxe/English.inf
Sample/Bus/LinuxThunk/BlockIo/Dxe/LinuxBlockIo.inf
Sample/Bus/LinuxThunk/Console/Dxe/LinuxConsole.inf
Sample/Bus/LinuxThunk/SerialIo/Dxe/LinuxSerialIo.inf
Sample/Bus/LinuxThunk/SimpleFileSystem/Dxe/LinuxSimpleFileSystem.inf
Sample/Bus/LinuxThunk/LinuxBusDriver/Dxe/LinuxBusDriver.inf
Sample/Bus/LinuxThunk/LinuxThunk/Dxe/LinuxThunk.inf

#
# In EFI mode,  GRAPHICS_INF = Sample/Bus/LinuxThunk/Uga/Dxe/LinuxUga.inf
# In UEFI mode, GRAPHICS_INF = Sample/Bus/LinuxThunk/Gop/Dxe/LinuxGop.inf
$(GRAPHICS_INF)

Sample/Platform/Linux32/Dxe/Linux32Platform/MiscSubclass/MiscSubclassDriver.inf
Sample/Bus/Pci/IdeBus/Dxe/IdeBus.inf                                          
#Sample/Bus/Pci/IdeController/Dxe/IdeController.inf                                          
Sample/Bus/Pci/Uhci/Dxe/Uhci.inf                                              
#Sample/Bus/Pci/Ehci/Dxe/Ehci.inf                                              
#Sample/Bus/Pci/EhciRouting/Dxe/EhciRouting.inf                                              
#Sample/Bus/Pci/VgaMiniPort/Dxe/VgaMiniPort.inf
Sample/Bus/Pci/Undi/RuntimeDxe/Undi.inf                                       
$(ATAPI_PASS_THRU_INF)
Sample/Bus/Scsi/ScsiBus/Dxe/ScsiBus.inf
Sample/Bus/Scsi/ScsiDisk/Dxe/ScsiDisk.inf
Sample/Bus/Usb/UsbBot/Dxe/UsbBot.inf                                          
Sample/Bus/Usb/UsbBus/Dxe/UsbBus.inf                                          
Sample/Bus/Usb/UsbCbi/Dxe/Cbi0/UsbCbi0.inf                                    
Sample/Bus/Usb/UsbCbi/Dxe/Cbi1/UsbCbi1.inf                                    
Sample/Bus/Usb/UsbKb/Dxe/UsbKb.inf                                            
Sample/Bus/Usb/UsbMassStorage/Dxe/UsbMassStorage.inf                          
Sample/Bus/Usb/UsbMouse/Dxe/UsbMouse.inf                                      
Sample/Universal/Network/PxeBc/Dxe/BC.inf                                     
Sample/Universal/Network/PxeDhcp4/Dxe/PxeDhcp4.inf                               
Sample/Universal/Network/Snp32_64/Dxe/SNP.inf                                 

#
# ISA Bus and Device
#
#Sample/Bus/Isa/IsaBus/Dxe/LightIsaBus.inf
#Sample/Bus/Isa/IsaBus/Dxe/IsaBusLite.inf
#Sample/Bus/Isa/IsaBus/Dxe/IsaBus.inf
#Sample/Bus/Isa/IsaFloppy/Dxe/IsaFloppyLite.inf
#Sample/Bus/Isa/IsaFloppy/Dxe/IsaFloppy.inf
#Sample/Bus/Isa/IsaSerial/Dxe/IsaSerialLite.inf
#Sample/Bus/Isa/IsaSerial/Dxe/IsaSerial.inf
#Sample/Bus/Isa/Ps2Keyboard/Dxe/Ps2keyboard.inf
#Sample/Bus/Isa/Ps2Mouse/Dxe/Ps2Mouse.inf

#
# UEFI network drivers.
#
Sample/Universal/Network/Mnp/Dxe/Mnp.inf                               FV=NULL
Sample/Universal/Network/Arp/Dxe/Arp.inf                               FV=NULL
Sample/Universal/Network/Ip4/Dxe/Ip4.inf                               FV=NULL
Sample/Universal/Network/Ip4Config/Dxe/Ip4Config.inf                   FV=NULL
Sample/Universal/Network/Udp4/Dxe/Udp4.inf                             FV=NULL
Sample/Universal/Network/Tcp4/Dxe/Tcp4.inf                             FV=NULL
Sample/Universal/Network/Dhcp4/Dxe/Dhcp4.inf                           FV=NULL
Sample/Universal/Network/Mtftp4/Dxe/Mtftp4.inf                         FV=NULL

#Sample/Universal/Network/SnpNt32/Dxe/SnpNt32.inf                       FV=NULL

#
# The following components are commented out for a variety of reasons.
# Components are listed in alphabetical order.
#

#
# Fat File System driver. Default is binary only 
#
#Other/Maintained/Universal/Disk/FileSystem/EnhancedFat/Dxe/Fat.inf


#
# Just define the working FV for the working block.
# This FV primarily contains private data of FTW and runtime updatable
# data such as variable and event log. It does not contain any FFS file
#
DEFINE NONFFS_FV=NvStorage

[=============================================================================]

