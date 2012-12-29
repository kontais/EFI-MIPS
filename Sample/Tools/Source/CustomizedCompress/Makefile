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

#
# Define some macros we use here. Should get rid of them someday and 
# get rid of the extra level of indirection.
#
TARGET_NAME        = CustomizedCompress
TARGET_SOURCE_DIR  = $(EDK_TOOLS_SOURCE)/$(TARGET_NAME)
COMMON_SOURCE      = $(EDK_TOOLS_COMMON)

#
# Common information
#

TARGET_LIB = $(EDK_TOOLS_OUTPUT)/CustomizedCompress.a

OBJECTS = $(EDK_TOOLS_OUTPUT)/CustomizedCompress.o 

#
# Build targets
#

all: $(TARGET_LIB)

#
# Object targets
#
 
$(EDK_TOOLS_OUTPUT)/CustomizedCompress.o: $(TARGET_SOURCE_DIR)/CustomizedCompress.c $(EDK_SOURCE)/Foundation/Include/EfiCommon.h
	$(CC) $(C_FLAGS) $(TARGET_SOURCE_DIR)/CustomizedCompress.c -o $(EDK_TOOLS_OUTPUT)/CustomizedCompress.o

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
	$(RM) $(EDK_TOOLS_OUTPUT)/CustomizedCompress.*
	$(RM) $(TARGET_LIB)
