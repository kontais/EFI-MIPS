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
#    makefile
#  
#  Abstract:
#  
#    Makefile for building the EFI VFR compiler
#    
#--*/  


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

.SUFFIXES :

TARGET_NAME       = VfrCompile
ANTLR_H           = $(PCCTS_DIR)/h
#C_FLAGS_PCCTS     = -I. -I$(ANTLR_H) /Zi /Fd$(EDK_TOOLS_OUTPUT)/$(TARGET_NAME)Obj /WX /Od /D _CRT_SECURE_NO_DEPRECATE $(VERSION_FLAGS)
C_FLAGS_PCCTS     = -I. -I$(ANTLR_H) -Wall -D_CRT_SECURE_NO_DEPRECATE $(VERSION_FLAGS)
C_FLAGS_PCCTS     +=

LINK_FLAGS_PCCTS  =
LIBS              = $(EDK_TOOLS_OUTPUT)/Common.a

# 
# Define the EFI output and source directories.
#
ETO               = $(EDK_TOOLS_OUTPUT)
SRC               = $(EDK_TOOLS_SOURCE)/$(TARGET_NAME)
TARGET_EXE        = $(ETO)/$(TARGET_NAME)

#
# Add deeper lookahead with -ck 3
#
ANTLR_FLAGS       = -CC -e3 -ck 3
DLG_FLAGS         = -C2 -i -CC

#
# Define paths for include files
#
INC = -I$(SRC) 
INC += -I$(EDK_SOURCE)/Foundation/Include/Mips32
INC += -I$(EDK_SOURCE)/Foundation/Efi/Include
INC += -I$(EDK_SOURCE)/Foundation/Framework/Include
INC += -I$(EDK_SOURCE)/Foundation/Include/IndustryStandard
INC += -I$(EDK_SOURCE)/Foundation/         \
             -I$(EDK_SOURCE)/Foundation/Core/Dxe \
             -I$(EDK_SOURCE)/Foundation/Efi      \
             -I$(EDK_SOURCE)/Foundation/Framework
INC += -I$(EDK_TOOLS_SOURCE)/Common
INC += -I$(EDK_SOURCE)/Foundation/Include 

DLG_FILE          = parser.dlg
SCAN_FILE         = DLGLexer
PARSER_FILE       = EfiVfrParser

#
# Create a list of include dependencies 
#
INC_DEPS += $(SRC)/EfiVfr.h
INC_DEPS += $(EDK_SOURCE)/Foundation/Framework/Include/EfiInternalFormRepresentation.h

#
# This is the grammer file for our project
#
GRAMMER_FILE  = $(SRC)/$(TARGET_NAME).g

#
# If we utilize a separate token file, define it here.
#
#TOKEN_FILE    = 

    
OBJECTS =       $(ETO)/$(TARGET_NAME).o \
                $(ETO)/$(PARSER_FILE).o \
                $(ETO)/AParser.o \
                $(ETO)/DLexerBase.o \
                $(ETO)/ATokenBuffer.o \
                $(ETO)/$(SCAN_FILE).o \
                $(ETO)/VfrServices.o

#
# Per the Language Translation Using PCCTS and C++ Reference Guide, page 109,
# these are the outputs of ANTLR and DLG
#                
ANTLR_SPAWN =   $(ETO)/$(TARGET_NAME).cpp \
                $(ETO)/$(PARSER_FILE).cpp \
	              $(ETO)/$(PARSER_FILE).h \
                $(ETO)/$(DLG_FILE) \
                $(ETO)/tokens.h 

DLG_SPAWN =     $(ETO)/$(SCAN_FILE).cpp \
                $(ETO)/$(SCAN_FILE).h


#
# Default target
#
all : $(TARGET_EXE)

#
# All antlr-generated files depend on the .g grammer file. Use the -o
# option to emit them to the appropriate output directory.
#
$(ANTLR_SPAWN) : $(GRAMMER_FILE) $(INC_DEPS)
	$(ANTLR) $(ANTLR_FLAGS) -o $(ETO) $(GRAMMER_FILE)

$(ETO)/$(TARGET_NAME).o : $(ETO)/$(TARGET_NAME).cpp $(DLG_SPAWN) $(INC_DEPS)
	$(CXX) -c $(C_FLAGS_PCCTS) -o $@ $(INC) $(ETO)/$(TARGET_NAME).cpp

$(ETO)/$(SCAN_FILE).o : $(ETO)/$(SCAN_FILE).cpp $(DLG_SPAWN) $(INC_DEPS)
	$(CXX) -c $(C_FLAGS_PCCTS) -o $@ $(INC) $(ETO)/$(SCAN_FILE).cpp

$(ETO)/$(PARSER_FILE).o : $(ETO)/$(PARSER_FILE).cpp $(ETO)/$(PARSER_FILE).h $(DLG_SPAWN) $(INC_DEPS)
	$(CXX) -c $(C_FLAGS_PCCTS) -o $@ $(INC) $(ETO)/$(PARSER_FILE).cpp

$(DLG_SPAWN) : $(ETO)/$(DLG_FILE) $(INC_DEPS)
	$(DLG) $(DLG_FLAGS) -o $(ETO) $(ETO)/$(DLG_FILE)

$(ETO)/AParser.o : $(ANTLR_H)/AParser.cpp
	$(CXX) -c $(C_FLAGS_PCCTS) -o $@ $(ANTLR_H)/AParser.cpp

$(ETO)/ATokenBuffer.o : $(ANTLR_H)/ATokenBuffer.cpp
	$(CXX) -c $(C_FLAGS_PCCTS) -o $@ $(ANTLR_H)/ATokenBuffer.cpp

$(ETO)/DLexerBase.o : $(ANTLR_H)/DLexerBase.cpp
	$(CXX) -c $(C_FLAGS_PCCTS) -o $@ $(ANTLR_H)/DLexerBase.cpp

$(ETO)/VfrServices.o : $(SRC)/VfrServices.cpp $(SRC)/VfrServices.h $(INC_DEPS)
	$(CXX) -c $(C_FLAGS_PCCTS) $(INC) -o $@ $(SRC)/VfrServices.cpp

#
# Add Binary Build description for this tools.
#

ifeq ($(EFI_BINARY_TOOLS),YES)
ifneq ($(wildcard $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME)),)
$(TARGET_EXE): $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME)
	$(CP) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME) $(TARGET_EXE)
else
$(TARGET_EXE) : $(OBJECTS) $(LIBS)
	$(CXX) -DDEBUG -o $(TARGET_EXE) $(LINK_FLAGS_PCCTS) $(OBJECTS) $(LIBS)
	@if [ -d $(EFI_PLATFORM_BIN)/Tools ] ; then \
	$(ECHO) exist $(EFI_PLATFORM_BIN)/Tools > $(NULL) ; \
	else \
	$(MKDIR) $(EFI_PLATFORM_BIN)/Tools ; \
	fi
	@if [ -e $(TARGET_EXE) ] ; then $(CP) $(TARGET_EXE) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME) ; fi
endif
else
$(TARGET_EXE) : $(OBJECTS) $(LIBS)
	$(CXX) -DDEBUG -o $(TARGET_EXE) $(LINK_FLAGS_PCCTS) $(OBJECTS) $(LIBS)
	@if [ -d $(EFI_PLATFORM_BIN)/Tools ] ; then \
	$(ECHO) exist $(EFI_PLATFORM_BIN)/Tools > $(NULL) ; \
	else \
	$(MKDIR) $(EFI_PLATFORM_BIN)/Tools ; \
	fi
	@if [ -e $(TARGET_EXE) ] ; then $(CP) $(TARGET_EXE) $(EFI_PLATFORM_BIN)/Tools/$(TARGET_NAME) ; fi
endif

clean:
  
