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
#  Module Name:
#  
#    Makefile
#  
#  Abstract:
#  
#    makefile for building the GUID check utility.
#    
#--*/

#
# Make sure environmental variable EDK_SOURCE is set
#
ifndef EDK_SOURCE
$(error EDK_SOURCE environmental variable not set )
endif

#
# Do this if you want to compile from this directory
#
ifndef TOOLCHAIN
TOOLCHAIN = TOOLCHAIN_GCC
endif

include $(BUILD_DIR)/PlatformTools.env

#
# Target specific information
#

TARGET_NAME         = GuidChk
TARGET_SRC_DIR      = $(EDK_TOOLS_SOURCE)/$(TARGET_NAME)
TARGET_EXE          = $(EDK_TOOLS_OUTPUT)/GuidChk

LIBS                += $(EDK_TOOLS_OUTPUT)/Common.a

#
# Build targets
#
all: $(TARGET_EXE)

INC_DEPS   += $(TARGET_SRC_DIR)/FileSearch.h
#INC_DEPS  += $(TARGET_SRC_DIR)/CommonUtils.h
#INC_DEPS  += $(TARGET_SRC_DIR)/UtilsMsgs.h

OBJECTS   = $(EDK_TOOLS_OUTPUT)/GuidChk.o  \
            $(EDK_TOOLS_OUTPUT)/FileSearch.o \
            $(EDK_TOOLS_OUTPUT)/GuidList.o \
            $(EDK_TOOLS_OUTPUT)/UtilsMsgs.o
#
# Compile each source file
#
$(EDK_TOOLS_OUTPUT)/GuidChk.o : $(TARGET_SRC_DIR)/GuidChk.c $(INC_DEPS)
	$(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)/GuidChk.c -o $@

$(EDK_TOOLS_OUTPUT)/FileSearch.o : $(TARGET_SRC_DIR)/FileSearch.c $(INC_DEPS)
	$(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)/FileSearch.c -o $@

$(EDK_TOOLS_OUTPUT)/GuidList.o : $(TARGET_SRC_DIR)/GuidList.c $(INC_DEPS)
	$(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)/GuidList.c -o $@

$(EDK_TOOLS_OUTPUT)/UtilsMsgs.o : $(TARGET_SRC_DIR)/UtilsMsgs.c $(INC_DEPS)
	$(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)/UtilsMsgs.c -o $@

#
# Add Binary Build description for this tool.
#

ifeq ($(EFI_BINARY_TOOLS),YES)
ifneq ($(wildcard $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME)),)
$(TARGET_EXE): $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME)
	$(CP) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME) $(TARGET_EXE)
else
$(TARGET_EXE) : $(OBJECTS)
	$(CC)  -o $(TARGET_EXE) $(OBJECTS) $(LIBS)
	@if [ -d $(EFI_PLATFORM_BIN)/Tools ] ; then \
	$(ECHO) exist $(EFI_PLATFORM_BIN)/Tools > $(NULL) ; \
	else \
	$(MKDIR) $(EFI_PLATFORM_BIN)/Tools ; \
	fi
	@if [ -e $(TARGET_EXE) ] ; then $(CP) $(TARGET_EXE) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME) ; fi
endif
else
$(TARGET_EXE) : $(OBJECTS)
	$(CC)  -o $(TARGET_EXE) $(OBJECTS) $(LIBS)
	@if [ -d $(EFI_PLATFORM_BIN)/Tools ] ; then \
	$(ECHO) exist $(EFI_PLATFORM_BIN)/Tools > $(NULL) ; \
	else \
	$(MKDIR) $(EFI_PLATFORM_BIN)/Tools ; \
	fi
	@if [ -e $(TARGET_EXE) ] ; then $(CP) $(TARGET_EXE) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME) ; fi
endif

clean:
	$(RM) $(EDK_TOOLS_OUTPUT)/$(TARGET_NAME)*
	$(RM) $(EDK_TOOLS_OUTPUT)/FileSearch.*
