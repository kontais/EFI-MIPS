#/*++
#
#  Copyright (c) 2004 - 2007, Intel Corporation                                                         
#  Copyright (c) 2009, kontais
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
#    Makefile
#  
#  Abstract:
#   
#    This makefile is used to build the Linux emulation.
#   
#--*/

#
# Everything depends on EDK_SOURCE & EDK_TOOLS_PATH. Make sure it's defined
#
ifndef EDK_SOURCE
$(error EDK_SOURCE environmental variable not set )
endif

ifndef EDK_TOOLS_PATH
$(warning EDK_TOOLS_PATH environmental variable not set, default setting used)
endif

#
# Global GNU make setting
#
MAKEFLAGS += -rR --no-print-directory

#
# Set the processor type
#
PROCESSOR = Mips32

#
# Define a toolchain so we can pick up the basic defines from
# the EFI tool setup file.
#
TOOLCHAIN = TOOLCHAIN_GCC

include PlatformTools.env

#
# Libs to link together the Linux console app
#
SEC_OBJECTS =                                                     \
  $(PROCESSOR)/Sample/Cpu/LinuxThunk/Sec/SecMain.o                \
  $(PROCESSOR)/Sample/Cpu/LinuxThunk/Sec/Mips32/SecSwitchStacks.o \
  $(PROCESSOR)/Sample/Cpu/LinuxThunk/Sec/FwVol.o                  \
  $(PROCESSOR)/Sample/Cpu/LinuxThunk/Sec/LinuxThunk.o             \
  $(PROCESSOR)/Sample/Cpu/LinuxThunk/Sec/UgaX11.o                 \
  $(PROCESSOR)/EdkLinux32PpiLib.a                                 \
  $(PROCESSOR)/EdkFrameworkPpiLib.a                               \
  $(PROCESSOR)/PeiHobLib.a                                        \
  $(PROCESSOR)/PeiLib.a                                           \
  $(PROCESSOR)/CustomizedDecompress.a                             \
  $(PROCESSOR)/EfiCommonLib.a                                     \
  $(PROCESSOR)/EdkGuidLib.a                                       \
  $(PROCESSOR)/EdkFrameworkGuidLib.a                              \
	$(PROCESSOR)/EfiGuidLib.a

#
# Linux Libaray
#
LINUX_LIBS = -ldl -lXext -lX11

#
# Default target
#
all : customer

#
# Target to build customer config
#
#customer : tempstamp_file
#customer : tempstamp_file build_tools
#customer : tempstamp_file build_tools makefiles
#customer : tempstamp_file build_tools makefiles builds
#customer : tempstamp_file build_tools makefiles builds fds
customer : tempstamp_file build_tools makefiles builds fds $(PROCESSOR)/SecMain

#
# Target to build everything quickly
#
fast : build_tools fast_makefiles builds fds $(PROCESSOR)/SecMain

#
# Create a file to store the date and time, and used by SetStamp
#
tempstamp_file:
ifdef DATE_STAMP
  ifdef TIME_STAMP
	  $(ECHO) $(DATE_STAMP) $(TIME_STAMP) > GenStamp.txt
  else
	  @$(RM) GenStamp.txt
  endif
else
	@$(RM) GenStamp.txt
endif

#
# Make all the EFI build tools by calling the makefile
#
build_tools :
	@if [ -d $(EDK_TOOLS_OUTPUT) ] ; 	then \
	$(ECHO) exist $(EDK_TOOLS_OUTPUT) > $(NULL) ; \
	else \
	$(MKDIR) $(EDK_TOOLS_OUTPUT) ; \
	fi
#	@$(ECHO) $(EDK_TOOLS_OUTPUT)
#	@$(ECHO) $(EDK_TOOLS_BIN)
	@if [ -e $(EDK_TOOLS_BIN)/*.* ] ; then \
	$(CP) $(EDK_TOOLS_BIN)/*.* $(EDK_TOOLS_OUTPUT) ; \
	fi
	$(MAKE) -f $(EDK_TOOLS_SOURCE)/Makefile PROCESSOR=$(PROCESSOR) BUILD_DIR=$(BUILD_DIR)
	$(GUIDCHK) -b $(EDK_TOOLS_OUTPUT)/GuidDatabase.txt  

#
# Build our executable file by linking the PEI object files
#
$(PROCESSOR)/SecMain: $(SEC_OBJECTS)
	$(CC) -o $(PROCESSOR)/SecMain $(LINUX_LIBS) $(SEC_OBJECTS)

#
# Run ProcessDsc on our main description files to generate the
# makefiles and package files. Specify -v option for more verbose output.
# Specify "-xref filename" to generate a cross-reference file of 
# basename-to-fileguid that can be used by VolInfo.
#
makefiles : 
	$(PROCESSDSC)                                    \
    -d EFI_SOURCE=$(EDK_SOURCE)                    \
    -d EDK_SOURCE=$(EDK_SOURCE)                    \
    -d PROCESSOR=$(PROCESSOR)                      \
    -d PROJECT_NAME=$(PROJECT_NAME)                \
    -d COMPRESS_METHOD=$(COMPRESS_METHOD)          \
    -d OEMTOOLPATH=$(EDK_TOOLS_OUTPUT)             \
    -d SHELL_INF=$(SHELL_INF)                      \
    -d UEFI_PREFIX=$(UEFI_PREFIX)                  \
    -d CAPSULE_INF=$(CAPSULE_INF)                  \
    -d ATAPI_PASS_THRU_INF=$(ATAPI_PASS_THRU_INF)  \
    -d DEVPATH_INF=$(DEVPATH_INF)                  \
    -d GRAPHICS_INF=$(GRAPHICS_INF)                \
    -g $(EDK_TOOLS_OUTPUT)/GuidDatabase.txt        \
    -xref FV/Guid.xref                             \
    $(PROJECT_NAME).dsc
    
#
# Build faster by not building components with FV=NULL. Override in the
# DSC file by adding SKIP_FV_NULL=0 on the component lines you want built
# but still have FV=NULL. For example:
#
# Dxe/Drivers/MyDriver/MyDriver.inf  FV=NULL  SKIP_FV_NULL=0
#
fast_makefiles : 
	$(PROCESSDSC)                                    \
    -d EFI_SOURCE=$(EDK_SOURCE)                    \
    -d EDK_SOURCE=$(EDK_SOURCE)                    \
    -d PROCESSOR=$(PROCESSOR)                      \
    -d PROJECT_NAME=$(PROJECT_NAME)                \
    -d COMPRESS_METHOD=$(COMPRESS_METHOD)          \
    -d OEMTOOLPATH=$(EDK_TOOLS_OUTPUT)             \
    -d SHELL_INF=$(SHELL_INF)                      \
    -d UEFI_PREFIX=$(UEFI_PREFIX)                  \
    -d CAPSULE_INF=$(CAPSULE_INF)                  \
    -d ATAPI_PASS_THRU_INF=$(ATAPI_PASS_THRU_INF)  \
    -d DEVPATH_INF=$(DEVPATH_INF)                  \
    -d GRAPHICS_INF=$(GRAPHICS_INF)                \
    -g $(EDK_TOOLS_OUTPUT)/GuidDatabase.txt        \
    -xref FV/Guid.xref                             \
    -d SKIP_FV_NULL=1                              \
    $(PROJECT_NAME).dsc

#
# Create the guid-to-basename cross reference file of protocols
# used by volinfo for decoding dependency expressions.
#
protocols.xref : 
#  -@if not exist fv mkdir FV
#  cd $(EDK_SOURCE)
#  $(GUIDCHK) -g -u $(BUILD_TARGET_DIRECTORY) -e inf -x > $(BUILD_DIR)/Fv/protocols.xref
#  cd $(BUILD_DIR)

#
# Call all the makefiles that were generated by ProcessDsc. ProcessDsc
# creates the makefile.out that calls all the component makefiles.
#
builds :
	$(MAKE) -f makefile.out

#
# Build the FV and FD images.
#
fds :
	$(CAT) FV/FvRecovery.fv FV/NVSTORAGE.fv > FV/FvRecovery.fd


#
# Useful targets
#

#
# Define a pseudo-target that skips over compiling the build tools and
# also skips creation of the makefiles for the components. The user
# has to be aware that any changes to INF files renders this target
# invalid.
#
quick : builds fds $(PROCESSOR)/SecMain

#
# Define a pseudo-target to start building with the libraries or with the 
# components.
#
libraries :
	$(MAKE) -f makefile.out libraries

components :
	$(MAKE) -f makefile.out components

#
# Make a pseudo-target to simplify running of the NT emulation. 
#
#if "$(EFI_WIN_NT_FILE_SYSTEM)" == ""
#EFI_WIN_NT_FILE_SYSTEM = $(EDK_SOURCE)/Other/Maintained/Application/$(UEFI_PREFIX)Shell/bin/ia32/Apps
#else
#EFI_WIN_NT_FILE_SYSTEM = $(EFI_WIN_NT_FILE_SYSTEM)!$(EDK_SOURCE)/Other/Maintained/Application/$(UEFI_PREFIX)Shell/bin/ia32/Apps
#endif

run :
#  set EFI_WIN_NT_FILE_SYSTEM=$(EFI_WIN_NT_FILE_SYSTEM)
	./$(PROCESSOR)/SecMain

#
# Make browse info file
#
bsc:
#  - dir /s /b *.sbr > $(PROCESSOR)/temp.rm
#  $(BSCMAKE) /n /o $(PROCESSOR)/$(PROJECT_NAME).bsc "@$(PROCESSOR)/temp.rm"

#
# Create a target to clean up all build directories
#
clean :
	@$(RM) *.bak           > $(NULL)
	@$(RM) makefile.out    > $(NULL)
	@$(RM) module.*        > $(NULL)
	@$(RMDIR) $(PROCESSOR) > $(NULL)
	@$(RMDIR) ebc          > $(NULL)
	@$(RMDIR) FV           > $(NULL)
	@$(RMDIR) Tools        > $(NULL)
	@$(ECHO) All targets cleaned

#
# Clean up all build directories except for tools.
#
cleanbuilds :
	@$(RM) *.bak           > $(NULL)
	@$(RM) makefile.out    > $(NULL)
	@$(RM) module.*        > $(NULL)
	@$(RMDIR) $(PROCESSOR) > $(NULL)
	@$(RMDIR) ebc          > $(NULL)
	@$(RMDIR) FV           > $(NULL)
	@$(ECHO) All modules cleaned

#
# Clean up only the tools
#
cleantools :
	@$(RMDIR) Tools        > $(NULL)
	@$(ECHO) All tools cleaned

#
# Rebuild target, clean and build all
#
rebuild : clean all

#
# cleanall target to clean all build directories.
#
cleanall : clean
