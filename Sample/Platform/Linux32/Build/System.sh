#/*++
#  
#  Copyright (c) 2004 - 2006, Intel Corporation                                                         
#  All rights reserved. This program and the accompanying materials                          
#  are licensed and made available under the terms and conditions of the BSD License         
#  which accompanies this distribution.  The full text of the license may be found at        
#  http://opensource.org/licenses/bsd-license.php                                            
#                                                                                            
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
#  
#  Module Name:
#  
#    system.cmd
#  
#  Abstract:
#  
#    Script file to setup environment for running the NT emulation.
#    
#--*/  
#
# This file is used to export envirnoment variables for the Win NT build.
# These variables are used to define the (virtual) hardware configuration
# of the NT environment
#
# A ! can be used to seperate multiple instances in a variable. Each 
# instance represents a seperate hardware device. 
#
# EFI_LINUX_SERIAL_PORT    - maps physical serial ports
# EFI_LINUX_GOP            - Builds GraphicsOutput Window
# EFI_LINUX_UGA            - Builds UGA Window
# EFI_LINUX_FILE_SYSTEM    - map a local directory to a file system
# EFI_FIRMWARE_VOLUMES      - File name of FDs, ! supported
# EFI_MEMORY_SIZE           - size of memory in megabytes, ! supported

# These variables only support a single instance
#
# EFI_LINUX_CONSOLE        - make a logical comand line window (only one!)
# EFI_BOOT_MODE             - decimal representation for the boot mode
# EFI_CPU_MODEL             - Customize CPU model
# EFI_CPU_SPEED             - Customize CPU speed in MHz

#
# EFI_LINUX_PHYSICAL_DISKS - maps to drives on your system
# EFI_LINUX_VIRTUAL_DISKS  - maps to an device emulated by a file
#
#  <F>ixed       - Fixed disk like a hard drive.
#  <R>emovable   - Removable media like a floppy or CD-ROM.
#  Read <O>nly   - Write protected device.
#  Read <W>rite  - Read write device.
#  <block count> - Decimal number of blocks a device supports.
#  <block size>  - Decimal number of bytes per block.
#
#  NT envirnonment variable contents. '<' and '>' are not part of the variable, 
#  they are just used to make this help more readable. There should be no 
#  spaces between the ';'. Extra spaces will break the variable. A '!' is  
#  used to seperate multiple devices in a variable.
#
#  EFI_LINUX_VIRTUAL_DISKS = 
#    <F | R><O | W>;<block count>;<block size>[!...]
#
#  EFI_LINUX_PHYSICAL_DISKS =
#    <drive letter>:<F | R><O | W>;<block count>;<block size>[!...]
#
#  Virtual Disks: These devices use a file to emulate a hard disk or removable
#                 media device. 
#                 
#    Thus a 20 MB emulated hard drive would look like:
#    EFI_LINUX_VIRTUAL_DISKS=FW;40960;512
#
#    A 1.44MB emulated floppy with a block size of 1024 would look like:
#    EFI_LINUX_VIRTUAL_DISKS=RW;1440;1024
#
#  Physical Disks: These devices use NT to open a real device in your system
#	 Please be very careful you can do a lot of damage to your system with
#    this feature. It's a good idea to start out with a CD as it's read only.
#
#    Thus a 120 MB floppy would look like:
#    EFI_LINUX_PHYSICAL_DISKS=B:RW;245760;512
#
#    Thus a standard CD-ROM would look like:
#    EFI_LINUX_PHYSICAL_DISKS=Z:RO;307200;2048


#
# Map a: as a 1.44MB Floppy and a 128 MB USB Disk
#  drive letters may be different on your machine
#
export EFI_LINUX_PHYSICAL_DISKS="a:RW;2880;512!e:RW;262144;512"

#
# Map a file as a 20 MB hard disk. 
#
export EFI_LINUX_VIRTUAL_DISKS="FW;40960;512"

#
# Map COM1 and COM2
#

export EFI_LINUX_SERIAL_PORT="COM1!COM2"


#
# Make Win NT Windows. Only one supported today. Mostly obsolete with
#  UGA consoles.
#
# export EFI_LINUX_CONSOLE="Bus Driver Console Window"

export EFI_LINUX_GOP="Graphics Output Window 1!Graphics Output Window 2"
#export EFI_LINUX_UGA="UGA Window 1!UGA Window 2"
export EFI_LINUX_UGA="UGA Window 1"


#
# export FD information
#
export EFI_FIRMWARE_VOLUMES=../FV/FvRecovery.fd


#
# These directories will show up as file systems with no Block IO
#

export EFI_LINUX_FILE_SYSTEM=.


#
# The first region will be tested by PEI and handed to DXE
#  as tested memory. Regions are seperated by ! and other
#  regions are passed into DXE as untested memory
#

export EFI_MEMORY_SIZE="64!64"

export EFI_BOOT_MODE=1
export EFI_LINUX_CPU_MODEL="Loongson2F Processor Model"
export EFI_LINUX_CPU_SPEED=1000
