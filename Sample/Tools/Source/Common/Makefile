#/*++
#   
#  Copyright (c) 2004 - 2007, Intel Corporation                                                         
#  All rights reserved. This program and the accompanying materials                          
#  are licensed and made available under the terms and conditions of the BSD License         
#  which accompanies this distribution.  The full text of the license may be found at        
#  http://opensource.org/licenses/bsd-license.php                                            
#                                                                                            
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
#  
#  Module Name:  makefile
#   
#  Abstract:
#   
#    This file is used to build the EFI utility.
#   
#--*/

#
# Do this if you want to compile from this directory
#
ifndef TOOLCHAIN
TOOLCHAIN = TOOLCHAIN_GCC
endif

include $(BUILD_DIR)/PlatformTools.env

C_FLAGS += -D EDK_TOOLS_DEBUG

#
# Define some macros we use here. Should get rid of them someday and 
# get rid of the extra level of indirection.
#
TARGET_NAME        = Common
TARGET_SRC_DIR     = $(EDK_TOOLS_SOURCE)/$(TARGET_NAME)
COMMON_SOURCE      = $(EDK_TOOLS_COMMON)

#
# Common information
#

TARGET_LIB = $(EDK_TOOLS_OUTPUT)/Common.a
TARGET_SOURCE_DIR = $(COMMON_SOURCE)

OBJECTS = $(EDK_TOOLS_OUTPUT)/ParseInf.o          \
          $(EDK_TOOLS_OUTPUT)/EfiCompress.o       \
          $(EDK_TOOLS_OUTPUT)/TianoCompress.o     \
          $(EDK_TOOLS_OUTPUT)/Decompress.o        \
          $(EDK_TOOLS_OUTPUT)/Crc32.o             \
          $(EDK_TOOLS_OUTPUT)/CommonLib.o         \
          $(EDK_TOOLS_OUTPUT)/ElfLoader.o         \
          $(EDK_TOOLS_OUTPUT)/ElfRelocate.o       \
          $(EDK_TOOLS_OUTPUT)/FvLib.o             \
          $(EDK_TOOLS_OUTPUT)/EfiUtilityMsgs.o    \
          $(EDK_TOOLS_OUTPUT)/SimpleFileParsing.o \
          $(EDK_TOOLS_OUTPUT)/MyAlloc.o           \
          $(EDK_TOOLS_OUTPUT)/SysLib.o

#
# Build targets
#

all: $(TARGET_LIB)

#
# Object targets
#
 
$(EDK_TOOLS_OUTPUT)/ParseInf.o: $(TARGET_SOURCE_DIR)/ParseInf.c $(TARGET_SOURCE_DIR)/ParseInf.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/ParseInf.c -o $(EDK_TOOLS_OUTPUT)/ParseInf.o

$(EDK_TOOLS_OUTPUT)/MyAlloc.o: $(TARGET_SOURCE_DIR)/MyAlloc.c $(TARGET_SOURCE_DIR)/MyAlloc.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/MyAlloc.c -o $(EDK_TOOLS_OUTPUT)/MyAlloc.o

$(EDK_TOOLS_OUTPUT)/SysLib.o: $(TARGET_SOURCE_DIR)/SysLib.c $(TARGET_SOURCE_DIR)/SysLib.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/SysLib.c -o $(EDK_TOOLS_OUTPUT)/SysLib.o

$(EDK_TOOLS_OUTPUT)/EfiCompress.o: $(TARGET_SOURCE_DIR)/EfiCompress.c $(TARGET_SOURCE_DIR)/Compress.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/EfiCompress.c -o $(EDK_TOOLS_OUTPUT)/EfiCompress.o

$(EDK_TOOLS_OUTPUT)/TianoCompress.o: $(TARGET_SOURCE_DIR)/TianoCompress.c $(TARGET_SOURCE_DIR)/Compress.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/TianoCompress.c -o $(EDK_TOOLS_OUTPUT)/TianoCompress.o

$(EDK_TOOLS_OUTPUT)/Decompress.o: $(TARGET_SOURCE_DIR)/Decompress.c $(TARGET_SOURCE_DIR)/Decompress.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/Decompress.c -o $(EDK_TOOLS_OUTPUT)/Decompress.o

$(EDK_TOOLS_OUTPUT)/Crc32.o: $(TARGET_SOURCE_DIR)/Crc32.c $(TARGET_SOURCE_DIR)/Crc32.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/Crc32.c -o $(EDK_TOOLS_OUTPUT)/Crc32.o

$(EDK_TOOLS_OUTPUT)/CommonLib.o: $(TARGET_SOURCE_DIR)/CommonLib.c $(TARGET_SOURCE_DIR)/CommonLib.h $(EDK_SOURCE)/Foundation/Include/TianoCommon.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/CommonLib.c -o $(EDK_TOOLS_OUTPUT)/CommonLib.o

$(EDK_TOOLS_OUTPUT)/ElfLoader.o: $(EDK_SOURCE)/Foundation/Include/TianoCommon.h $(EDK_SOURCE)/Foundation/Library/Pei/PeiLib/ElfLoader.c
	$(CC)  -I $(EDK_TOOLS_OUTPUT) $(C_FLAGS) -I $(EDK_SOURCE)/Foundation/Library/Pei/Include $(EDK_SOURCE)/Foundation/Library/Pei/PeiLib/ElfLoader.c -o $(EDK_TOOLS_OUTPUT)/ElfLoader.o

$(EDK_TOOLS_OUTPUT)/ElfRelocate.o: $(EDK_SOURCE)/Foundation/Library/Pei/PeiLib/$(PROCESSOR)/ElfRelocate.c
	$(CC)  $(C_FLAGS) $(EDK_SOURCE)/Foundation/Library/Pei/PeiLib/$(PROCESSOR)/ElfRelocate.c -o $(EDK_TOOLS_OUTPUT)/ElfRelocate.o

$(EDK_TOOLS_OUTPUT)/FvLib.o: $(TARGET_SOURCE_DIR)/FvLib.c $(TARGET_SOURCE_DIR)/FvLib.h $(EDK_SOURCE)/Sample/Include/Efi2Linux.h $(EDK_SOURCE)/Foundation/Framework/Include/EfiFirmwareFileSystem.h $(EDK_SOURCE)/Foundation/Framework/Include/EfiFirmwareVolumeHeader.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/FvLib.c -o $(EDK_TOOLS_OUTPUT)/FvLib.o

$(EDK_TOOLS_OUTPUT)/EfiUtilityMsgs.o: $(TARGET_SOURCE_DIR)/EfiUtilityMsgs.c $(TARGET_SOURCE_DIR)/EfiUtilityMsgs.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/EfiUtilityMsgs.c -o $(EDK_TOOLS_OUTPUT)/EfiUtilityMsgs.o

$(EDK_TOOLS_OUTPUT)/SimpleFileParsing.o : $(TARGET_SOURCE_DIR)/SimpleFileParsing.c $(TARGET_SOURCE_DIR)/SimpleFileParsing.h
	$(CC)  $(C_FLAGS) $(TARGET_SOURCE_DIR)/SimpleFileParsing.c -o $(EDK_TOOLS_OUTPUT)/SimpleFileParsing.o

#
# Build LIB
#

#
# Add Binary Build description for this lib.
#

ifeq ($(EFI_BINARY_TOOLS), YES)
ifneq ($(wildcard $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME).a), )
$(TARGET_LIB): $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME).a
	$(CP) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME).a $(TARGET_LIB)
else
$(TARGET_LIB): $(OBJECTS)
	$(LIB) $(LIB_FLAGS) r $(TARGET_LIB) $(OBJECTS)
	@if [ -d $(EFI_PLATFORM_BIN)/Tools ] ; then \
	$(ECHO) exist $(EFI_PLATFORM_BIN)/Tools > $(NULL) ; \
	else \
	$(MKDIR) $(EFI_PLATFORM_BIN)/Tools ; \
	fi
	@if [ -e $(TARGET_LIB) ] ; then \
	$(CP) $(TARGET_LIB) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME).a ; \
	fi
endif
else
$(TARGET_LIB): $(OBJECTS)
	$(LIB) $(LIB_FLAGS) r $(TARGET_LIB) $(OBJECTS)
	@if [ -d $(EFI_PLATFORM_BIN)/Tools ] ; then \
	$(ECHO) exist $(EFI_PLATFORM_BIN)/Tools > $(NULL) ; \
	else \
	$(MKDIR) $(EFI_PLATFORM_BIN)/Tools ; \
	fi
	@if [ -e $(TARGET_LIB) ] ; then \
	$(CP) $(TARGET_LIB) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME).a ; \
	fi
endif

clean:
	$(RM) $(EDK_TOOLS_OUTPUT)/ParseInf.*
	$(RM) $(EDK_TOOLS_OUTPUT)/ParsePeim.*
	$(RM) $(EDK_TOOLS_OUTPUT)/EfiCompress.*
	$(RM) $(EDK_TOOLS_OUTPUT)/TianoCompress.*
	$(RM) $(EDK_TOOLS_OUTPUT)/Decompress.*
	$(RM) $(EDK_TOOLS_OUTPUT)/Crc32.*
	$(RM) $(EDK_TOOLS_OUTPUT)/CommonLib.*
	$(RM) $(EDK_TOOLS_OUTPUT)/FvLib.*
	$(RM) $(EDK_TOOLS_OUTPUT)/SysLib.*
	$(RM) $(TARGET_LIB)
