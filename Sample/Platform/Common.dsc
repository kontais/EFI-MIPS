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
#    Common.dsc
#
#  Abstract:
#
#    This is the build description file containing the platform
#    independent build instructions.  Platform specific instructions will
#    be prepended to produce the final build DSC file.
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
# These get emitted at the top of the generated master makefile. 
#
[=============================================================================]
[Makefile.out]
#
# From the [makefile.out] section of the DSC file
#
TOOLCHAIN = 
MAKE      = make

include $(BUILD_DIR)/PlatformTools.env

all : libraries fvs

[=============================================================================]
#
# These get expanded and dumped out to each component makefile after the
# component INF [defines] section gets parsed.
#
[=============================================================================]
[Makefile.Common]
#
# From the [Makefile.Common] section of the description file.
#
PROCESSOR        := $(PROCESSOR)
BASE_NAME        := $(BASE_NAME)
BUILD_NUMBER     := $(BUILD_NUMBER)
VERSION_STRING   := $(VERSION_STRING)
TOOLCHAIN        := TOOLCHAIN_$(PROCESSOR)
FILE_GUID        := $(FILE_GUID)
COMPONENT_TYPE   := $(COMPONENT_TYPE)
FV_DIR           := $(BUILD_DIR)/FV
PLATFORM         := $(PROJECT_NAME) 

#
# Define the global dependency files
#
ifneq ($(wildcard $(DEST_DIR)/$(BASE_NAME)StrDefs.h), )
INC_DEPS         += $(DEST_DIR)/$(BASE_NAME)StrDefs.h
endif
#ENV_DEPS         = $(ENV_DEPS) $(EDK_SOURCE)/Sample/CommonTools.env
#ENV_DEPS         = $(ENV_DEPS) $(BUILD_DIR)/PlatformTools.env
#ENV_DEPS         = $(ENV_DEPS) $(BUILD_DIR)/Config.env
ALL_DEPS         = $(INC_DEPS) $(ENV_DEPS)

ifneq ($(LANGUAGE), )
LANGUAGE_FLAGS    = -lang $(LANGUAGE)
endif

include $(BUILD_DIR)/PlatformTools.env

COMPONENT_PEI_TYPE = PIC_PEIM PE32_PEIM RELOCATABLE_PEIM COMBINED_PEIM_DRIVER

ifneq (,$(findstring $(COMPONENT_TYPE), $(COMPONENT_PEI_TYPE)))
DEPEX_TYPE = EFI_SECTION_PEI_DEPEX
else
DEPEX_TYPE = EFI_SECTION_DXE_DEPEX
endif

#
# Command flags for MAKEDEPS tool
#
DEP_FLAGS = -target $^ -o $@ $(INC) -ignorenotfound -q

[=============================================================================]
#
# These are the commands to compile source files. One of these blocks gets 
# emitted to the component's makefile for each source file. The section
# name is encoded as [Compile.$(PROCESSOR).source_filename_extension], where
# the source filename comes from the sources section of the component INF file.
#
# If the dependency list file already exists, then include it for this 
# source file. If it doesn't exist, then this is a clean build and the
# dependency file will get created below and the source file will get 
# compiled. 
#
# Current behavior is that the first clean build will not create dep files. 
# But the following second build has to create dep files before build source files.
# CREATEDEPS flag is used to judge whether current build is the second build or not.
#
#
[=============================================================================]
[Compile.Mips32.s,Compile.x64.s]

DEP_FILE    = $(DEST_DIR)/$(FILE)Asm.dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).o), )
DEP_TARGETS += $(DEST_DIR)/$(FILE)Asm.dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).o
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS) -asm

#
# Compile the file
#
$(DEST_DIR)/$(FILE).o : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(ASM) $(ASM_FLAGS) $(SOURCE_FILE_NAME)

[=============================================================================]
[Compile.Ipf.s]

DEP_FILE    = $(DEST_DIR)/$(FILE)S.dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).pro), )
DEP_TARGETS += $(DEST_DIR)/$(FILE)S.dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).pro
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS)

#
# Compile the file
#
$(DEST_DIR)/$(FILE).pro : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(CC) $(C_FLAGS_PRO) $(SOURCE_FILE_NAME) > $@

$(DEST_DIR)/$(FILE).o : $(DEST_DIR)/$(FILE).pro
	$(ASM) $(ASM_FLAGS) $(DEST_DIR)/$(FILE).pro

[=============================================================================]
[Compile.Mips32.c,Compile.Ipf.c,Compile.x64.c]

DEP_FILE    = $(DEST_DIR)/$(FILE).dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).o), )
DEP_TARGETS += $(DEST_DIR)/$(FILE).dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).o
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS)

#
# Compile the file
#
$(DEST_DIR)/$(FILE).o : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(CC) $(C_FLAGS) $(SOURCE_FILE_NAME)

[=============================================================================]
[Compile.Ebc.c]

DEP_FILE    = $(DEST_DIR)/$(FILE).dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).o), )
DEP_TARGETS += $(DEST_DIR)/$(FILE).dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).o
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS)

#
# Compile the file
#
$(DEST_DIR)/$(FILE).o : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(EBC_CC) $(EBC_C_FLAGS) $(SOURCE_FILE_NAME)

[=============================================================================]
#
# Commands for compiling a ".apr" Apriori source file.
#
[=============================================================================]
[Compile.Mips32.Apr,Compile.Ipf.Apr,Compile.Ebc.Apr,Compile.x64.Apr]
#
# Create the raw binary file. If you get an error on the build saying it doesn't
# know how to create the .apr file, then you're missing (or mispelled) the
# "APRIORI=" on the component lines in components section in the DSC file.
#
$(DEST_DIR)/$(BASE_NAME).bin : $(SOURCE_FILE_NAME)
	$(GENAPRIORI) -v -f $(SOURCE_FILE_NAME) -o $(DEST_DIR)/$(BASE_NAME).bin

$(DEST_DIR)/$(BASE_NAME).sec : $(DEST_DIR)/$(BASE_NAME).bin
	$(GENSECTION) -I $(DEST_DIR)/$(BASE_NAME).bin -O $(DEST_DIR)/$(BASE_NAME).sec -S EFI_SECTION_RAW

[=============================================================================]
[Build.Mips32.Apriori,Build.Ipf.Apriori,Build.Ebc.Apriori,Build.x64.Apriori]

all : $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).ffs

#
# Run GenFfsFile on the package file and .raw file to create the firmware file
#
$(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).ffs : $(DEST_DIR)/$(BASE_NAME).sec $(PACKAGE_FILENAME)
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
[Build.Mips32.Makefile,Build.Ipf.Makefile,Build.Ebc.Makefile,Build.x64.Makefile]

#
# Copy the makefile directly from the source directory, then make it
# writable so we can copy over it later if we try to.
#
$(DEST_DIR)/makefile.new : $(SOURCE_DIR)/makefile.new
	$(CP) $(SOURCE_DIR)/makefile.new $(DEST_DIR)/makefile.new
  attrib -r $(DEST_DIR)/makefile.new

#
# Make the all target, set some required macros.
#
call_makefile :
	$(MAKE) -f $(DEST_DIR)/makefile.new all   \
          SOURCE_DIR=$(SOURCE_DIR)          \
          BUILD_DIR=$(BUILD_DIR)            \
          FILE_GUID=$(FILE_GUID)            \
          DEST_DIR=$(DEST_DIR)              \
          PROCESSOR=$(PROCESSOR)            \
          TOOLCHAIN=TOOLCHAIN_$(PROCESSOR)  \
          BASE_NAME=$(BASE_NAME)            \
          PACKAGE_FILENAME=$(PACKAGE_FILENAME)

all : $(DEST_DIR)/makefile.new call_makefile

#
# Remove the generated temp and final files for this modules.
#
clean :
  @- $(MAKE) -f $(DEST_DIR)/makefile.new clean > $(NULL) 2>&1
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
#
# Instructions for building a component that uses a custom makefile. Encoding 
# is [build.$(PROCESSOR).$(BUILD_TYPE)].
#
# To build these components, simply call the makefile from the source 
# directory.
#
[=============================================================================]
[Build.Mips32.Custom_Makefile,Build.Ipf.Custom_Makefile,Build.Ebc.Custom_Makefile,Build.x64.Custom_Makefile]

#
# Just call the makefile from the source directory, passing in some
# useful info.
#
all : 
	$(MAKE) -f $(SOURCE_DIR)/makefile all    \
          SOURCE_DIR=$(SOURCE_DIR)         \
          BUILD_DIR=$(BUILD_DIR)           \
          DEST_DIR=$(DEST_DIR)             \
          FILE_GUID=$(FILE_GUID)           \
          PROCESSOR=$(PROCESSOR)           \
          TOOLCHAIN=TOOLCHAIN_$(PROCESSOR) \
          BASE_NAME=$(BASE_NAME)           \
          PLATFORM=$(PLATFORM)             \
          SOURCE_FV=$(SOURCE_FV)           \
          PACKAGE_FILENAME=$(PACKAGE_FILENAME)

#
# Remove the generated temp and final files for this modules.
#
clean :
  @- $(MAKE) -f $(SOURCE_DIR)/makefile clean > $(NULL) 2>&1
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
#
# These commands are used to build libraries
#
[=============================================================================]
[Build.Mips32.LIBRARY,Build.Ipf.LIBRARY,Build.x64.LIBRARY]
#
# LIB all the object files into to our target lib file. Put
# a dependency on the component's INF file in case it changes.
#
LIB_NAME = $(LIB_DIR)/$(BASE_NAME).a

$(LIB_NAME) : $(OBJECTS) $(LIBS) $(INF_FILENAME) $(ENV_DEPS)
	$(LIB) $(LIB_FLAGS)  r $@ $(OBJECTS) $(LIBS)

ifeq ($(CREATEDEPS),YES)
all : $(DEP_TARGETS)
	$(MAKE) -f $(MAKEFILE_NAME) all
else
all : $(LIB_NAME) $(DEP_TARGETS)
endif

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
[Build.Ebc.LIBRARY]
#
# LIB all the object files into to our target lib file. Put
# a dependency on the component's INF file in case it changes.
#
LIB_NAME = $(LIB_DIR)/$(BASE_NAME).a

$(LIB_NAME) : $(OBJECTS) $(LIBS) $(INF_FILENAME) $(ENV_DEPS)
   $(EBC_LIB) $(EBC_LIB_FLAGS) $(OBJECTS) $(LIBS) /OUT:$@

ifeq ($(CREATEDEPS),YES)
all : $(DEP_TARGETS)
	$(MAKE) -f $(MAKEFILE_NAME) all
else
all : $(LIB_NAME) $(DEP_TARGETS)
endif

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
#
# This is the Build.$(PROCESSOR).$(COMPONENT_TYPE) section that tells how to
# convert a firmware volume into an FV FFS file. Simply run it through
# GenFfsFile with the appropriate package file. SOURCE_FV must be defined
# in the component INF file Defines section.
#
[=============================================================================]
[Build.Mips32.FvImageFile,Build.x64.FvImageFile,Build.Ipf.FvImageFile]

all : $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).Fvi

#
# Run GenFfsFile on the package file and FV file to create the firmware 
# volume FFS file
#
$(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).Fvi : $(DEST_DIR)/$(SOURCE_FV)Fv.sec $(PACKAGE_FILENAME)
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
#
# Since many of the steps are the same for the different component types, we 
# share this section for BS_DRIVER, RT_DRIVER, .... and IFDEF the parts that 
# differ.  The entire section gets dumped to the output makefile.
#
[=============================================================================]
[Build.Mips32.BS_DRIVER|SECURITY_CORE|RT_DRIVER|SAL_RT_DRIVER|PE32_PEIM|PEI_CORE|PIC_PEIM|RELOCATABLE_PEIM|DXE_CORE|APPLICATION|COMBINED_PEIM_DRIVER, Build.Ipf.BS_DRIVER|RT_DRIVER|SAL_RT_DRIVER|PEI_CORE|PE32_PEIM|PIC_PEIM|DXE_CORE|APPLICATION|COMBINED_PEIM_DRIVER, Build.x64.BS_DRIVER|RT_DRIVER|SAL_RT_DRIVER|PE32_PEIM|PEI_CORE|PIC_PEIM|RELOCATABLE_PEIM|DXE_CORE|APPLICATION|COMBINED_PEIM_DRIVER]

ifeq ($(LOCALIZE),YES)

ifeq ($(EFI_GENERATE_HII_EXPORT),YES)
STRGATHER_FLAGS   += -hpk $(DEST_DIR)/$(BASE_NAME)Strings.hpk

#
# There will be one HII pack containing all the strings. Add that file
# to the list of HII pack files we'll use to create our final HII export file.
#
HII_PACK_FILES    += $(DEST_DIR)/$(BASE_NAME)Strings.hpk
LOCALIZE_TARGETS  += $(DEST_DIR)/$(BASE_NAME).hii
endif

$(DEST_DIR)/$(BASE_NAME).sdb : $(SDB_FILES) $(SOURCE_FILES)
	$(STRGATHER) -scan -vdbr $(STRGATHER_FLAGS) -od $(DEST_DIR)/$(BASE_NAME).sdb \
    -skipext .uni -skipext .h $(SOURCE_FILES)

$(DEST_DIR)/$(BASE_NAME)Strings.c : $(DEST_DIR)/$(BASE_NAME).sdb
	$(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)/$(BASE_NAME).sdb \
    -oc $(DEST_DIR)/$(BASE_NAME)Strings.c

$(DEST_DIR)/$(BASE_NAME)StrDefs.h : $(DEST_DIR)/$(BASE_NAME).sdb
	$(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)/$(BASE_NAME).sdb \
    -oh $(DEST_DIR)/$(BASE_NAME)StrDefs.h

$(DEST_DIR)/$(BASE_NAME)Strings.hpk : $(DEST_DIR)/$(BASE_NAME).sdb
	$(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)/$(BASE_NAME).sdb \
    -hpk $(DEST_DIR)/$(BASE_NAME)Strings.hpk

OBJECTS += $(DEST_DIR)/$(BASE_NAME)Strings.o

$(DEST_DIR)/$(BASE_NAME)Strings.o : $(DEST_DIR)/$(BASE_NAME)Strings.c $(INF_FILENAME) $(ALL_DEPS)
	$(CC) $(C_FLAGS) $(DEST_DIR)/$(BASE_NAME)Strings.c

LOCALIZE_TARGETS += $(DEST_DIR)/$(BASE_NAME)StrDefs.h

endif

#
# If we have any objects associated with this component, then we're
# going to build a local library from them.
#
ifndef OBJECTS
$(error No source files to build were defined in the INF file )
endif

TARGET_LOCAL_OBJECT  = $(DEST_DIR)/$(BASE_NAME)Local.o

#
# LIB all the object files into our (local) target lib file. Put
# a dependency on the component's INF file in case it changes.
#
$(TARGET_LOCAL_OBJECT) : $(OBJECTS)  $(INF_FILENAME) $(ENV_DEPS)
	$(LINK) $(LINK_FLAGS_DLL) -o $@ $(OBJECTS)

#
# Defines for standard intermediate files and build targets
#
TARGET_DLL      = $(BIN_DIR)/$(BASE_NAME).elf
TARGET_EFI      = $(BIN_DIR)/$(BASE_NAME).efi
TARGET_DPX      = $(DEST_DIR)/$(BASE_NAME).dpx
TARGET_UI       = $(DEST_DIR)/$(BASE_NAME).ui
TARGET_VER      = $(DEST_DIR)/$(BASE_NAME).ver
TARGET_MAP      = $(BIN_DIR)/$(BASE_NAME).map
TARGET_PDB      = $(BIN_DIR)/$(BASE_NAME).pdb
TARGET_SYM      = $(BIN_DIR)/$(BASE_NAME).sym
TARGET_TES      = $(DEST_DIR)/$(BASE_NAME).tes

#
# Target executable section extension depends on the component type.
# Only define "TARGET_DXE_DPX" if it's a combined peim driver.
#
ifeq ($(COMPONENT_TYPE),PIC_PEIM)
TARGET_PE32 = $(DEST_DIR)/$(BASE_NAME).pic
else
TARGET_PE32 = $(DEST_DIR)/$(BASE_NAME).pe32
endif

#
# Target FFS file extension depends on the component type
# Also define "TARGET_DXE_DPX" if it's a combined PEIM driver.
#
SUBSYSTEM = EFI_BOOT_SERVICE_DRIVER

ifeq ($(COMPONENT_TYPE),APPLICATION)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).app
SUBSYSTEM       = EFI_APPLICATION
else ifeq ($(COMPONENT_TYPE),PEI_CORE)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).pei
else ifeq ($(COMPONENT_TYPE),PE32_PEIM)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).pei
else ifeq ($(COMPONENT_TYPE),RELOCATABLE_PEIM)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).pei
else ifeq ($(COMPONENT_TYPE),PIC_PEIM)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).pei
else ifeq ($(COMPONENT_TYPE),COMBINED_PEIM_DRIVER)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).pei
TARGET_DXE_DPX  = $(DEST_DIR)/$(BASE_NAME).dpxd
else
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).dxe
endif

#
# Different methods to build section based on if PIC_PEIM
#
ifeq ($(COMPONENT_TYPE),PIC_PEIM)

$(TARGET_PE32) : $(TARGET_DLL)
	$(PE2BIN) $(TARGET_DLL) $(DEST_DIR)/$(BASE_NAME).TMP
#
# BUGBUG: Build PEIM header, needs to go away with new PEI.
#
  $(TEMPGENSECTION) -P $(SOURCE_DIR)/$(BASE_NAME).INF -I $(DEST_DIR)/$(BASE_NAME).TMP -O $(TARGET_PIC_PEI).tmp -M $(TARGET_MAP) -S EFI_SECTION_TYPE_NO_HEADER
	$(GENSECTION) -I $(TARGET_PIC_PEI).tmp -O $(TARGET_PE32) -S EFI_SECTION_PIC
	$(RM) $(DEST_DIR)/$(BASE_NAME).TMP

$(TARGET_TES) :
	$(TOUCH) $(TARGET_TES)

else

$(TARGET_PE32) : $(TARGET_EFI)
	$(GENSECTION) -I $(TARGET_EFI) -O $(TARGET_PE32) -S EFI_SECTION_PE32

$(TARGET_TES) : $(TARGET_EFI)
	$(GENSECTION) -I $(TARGET_EFI) -O $(TARGET_TES) -S EFI_SECTION_TE

#
# Run FWImage on the DLL to set it as an EFI image type.
#
$(TARGET_EFI) : $(TARGET_DLL) $(INF_FILENAME)
	$(FWIMAGE) -t 0 $(COMPONENT_TYPE) $(TARGET_DLL) $(TARGET_EFI)

endif

#
# Link all objects and libs to create the executable
# Generate ELF executable file is make sure no UND symbols
# loongson EFI use ELF relocatable file
#
$(TARGET_DLL) : $(TARGET_LOCAL_OBJECT) $(LIBS) $(INF_FILENAME) $(ENV_DEPS)
	$(LINK) $(LINK_FLAGS_EXE) -e $(IMAGE_ENTRY_POINT) \
	-o $(TARGET_DLL).exe  $(TARGET_LOCAL_OBJECT) $(LIBS) 
	$(LINK) $(LINK_FLAGS_DLL) -e $(IMAGE_ENTRY_POINT) \
	-o $(TARGET_DLL)  $(TARGET_LOCAL_OBJECT) $(LIBS) 
	$(SETSTAMP) $(TARGET_DLL) $(BUILD_DIR)/GenStamp.txt
ifeq ($(EFI_GENERATE_SYM_FILE),YES)
  if exist $(TARGET_PDB) $(PE2SYM) $(TARGET_PDB) $(TARGET_SYM)
endif

ifeq ($(EFI_ZERO_DEBUG_DATA),YES)
	$(ZERODEBUGDATA) $(TARGET_DLL)
endif

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
ifneq ($(BUILD_NUMBER), )
ifneq ($(VERSION_STRING), )
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
else
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
endif
else
$(TARGET_VER) : 
	touch $(TARGET_VER)
endif

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
ifneq ($(DPX_SOURCE), )
DPX_SOURCE_FILE = $(SOURCE_DIR)/$(DPX_SOURCE)
endif

ifneq ($(DPX_SOURCE_OVERRIDE), )
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
endif

ifneq ($(DPX_SOURCE_FILE), )
ifneq ($(wildcard $(DPX_SOURCE_FILE)), )
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
	$(CC) $(INC) $(VERSION_FLAGS) -E -P -x c $(DPX_SOURCE_FILE) > $@.tmp1
	$(GENDEPEX) -I $@.tmp1 -O $@.tmp2
	$(GENSECTION) -I $@.tmp2 -O $@ -S $(DEPEX_TYPE)
	$(RM) $@.tmp1 > $(NULL)
	$(RM) $@.tmp2 > $(NULL)
else
$(error Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist. )
endif
else
$(TARGET_DPX) : 
	touch $(TARGET_DPX)
endif

#
# Makefile entries for DXE DPX for combined PEIM drivers.
# If a DXE_DPX_SOURCE file was specified in the INF file, use it. Otherwise 
# create an empty file and use it as a DPX file.
#
ifeq ($(COMPONENT_TYPE),COMBINED_PEIM_DRIVER)
ifneq ($(DXE_DPX_SOURCE), )
ifneq ($(wildcard $(SOURCE_DIR)/$(DPX_SOURCE)), )
$(TARGET_DXE_DPX) : $(SOURCE_DIR)/$(DXE_DPX_SOURCE) $(INF_FILENAME)
	$(CC) $(INC) $(VERSION_FLAGS) -E -P -x c $(SOURCE_DIR)/$(DXE_DPX_SOURCE) > $@.tmp1
	$(GENDEPEX) -I $@.tmp1 -O $@.tmp2
	$(GENSECTION) -I $@.tmp2 -O $@ -S EFI_SECTION_DXE_DEPEX
	$(RM) $@.tmp1 > $(NULL)
	$(RM) $@.tmp2 > $(NULL)
else
$(error Dependency expression source file "$(SOURCE_DIR)/$(DXE_DPX_SOURCE)" does not 
exist. )
endif
else
$(TARGET_DXE_DPX) : 
	touch $(TARGET_DXE_DPX)
endif
endif

#
# Describe how to build the HII export file from all the input HII pack files.
# Use the FFS file GUID for the package GUID in the export file. Only used
# when multiple VFR share strings.
#
$(DEST_DIR)/$(BASE_NAME).hii : $(HII_PACK_FILES)
	$(HIIPACK) create -g $(FILE_GUID) -p $(HII_PACK_FILES) -o $(DEST_DIR)/$(BASE_NAME).hii

#
# If the build calls for creating an FFS file with the IFR included as
# a separate binary (not compiled into the driver), then build the binary
# section now. Note that the PACKAGE must be set correctly to actually get
# this IFR section pulled into the FFS file.
#
ifneq ($(HII_IFR_PACK_FILES), )

$(DEST_DIR)/$(BASE_NAME)IfrBin.sec : $(HII_IFR_PACK_FILES)
	$(HIIPACK) create -novarpacks -p $(HII_IFR_PACK_FILES) -o $(DEST_DIR)/$(BASE_NAME)IfrBin.hii
	$(GENSECTION) -I $(DEST_DIR)/$(BASE_NAME)IfrBin.hii -O $(DEST_DIR)/$(BASE_NAME)IfrBin.sec -S EFI_SECTION_RAW

BIN_TARGETS = $(BIN_TARGETS) $(DEST_DIR)/$(BASE_NAME)IfrBin.sec

endif

#
# Build a FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_PE32) $(TARGET_TES) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER) $(TARGET_DXE_DPX) $(PACKAGE_FILENAME)
#
# Some of our components require padding to align code
#
ifeq ($(PROCESSOR),IPF)
ifneq (,$(findstring $(COMPONENT_TYPE), PIC_PEIM PE32_PEIM RELOCATABLE_PEIM SECURITY_CORE PEI_CORE COMBINED_PEIM_DRIVER))
	$(CP) $(BIN_DIR)/Blank.pad $(DEST_DIR)
endif
endif
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

ifeq ($(CREATEDEPS),YES)
all : $(DEP_TARGETS)
	$(MAKE) -f $(MAKEFILE_NAME) all
else
all : $(LOCALIZE_TARGETS) $(BIN_TARGETS) $(TARGET_FFS_FILE) $(DEP_TARGETS)
endif

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
[Build.Mips32.TE_PEIM,Build.Ipf.TE_PEIM,Build.x64.TE_PEIM]
#
# Define the library file we'll build if we have any objects defined.
#
ifndef OBJECTS
TARGET_LOCAL_OBJECT  = $(DEST_DIR)/$(BASE_NAME)Local.o
#
# LIB all the object files into our (local) target lib file. Put
# a dependency on the component's INF file in case it changes.
#
$(TARGET_LOCAL_OBJECT) : $(OBJECTS)  $(INF_FILENAME) $(ENV_DEPS)
	$(LINK) $(LINK_FLAGS_DLL) -o $@ $(OBJECTS)

else
$(error No source files to build were defined in the INF file )
endif

#
# Defines for standard intermediate files and build targets
#
TARGET_DLL        = $(BIN_DIR)/$(BASE_NAME).elf
TARGET_EFI        = $(BIN_DIR)/$(BASE_NAME).efi
TARGET_DPX        = $(DEST_DIR)/$(BASE_NAME).dpx
TARGET_UI         = $(DEST_DIR)/$(BASE_NAME).ui
TARGET_VER        = $(DEST_DIR)/$(BASE_NAME).ver
TARGET_MAP        = $(BIN_DIR)/$(BASE_NAME).map
TARGET_PDB        = $(BIN_DIR)/$(BASE_NAME).pdb
TARGET_SYM        = $(BIN_DIR)/$(BASE_NAME).sym
TARGET_TE         = $(BIN_DIR)/$(BASE_NAME).te
TARGET_PE32       = $(DEST_DIR)/$(BASE_NAME).pe32
TARGET_TES        = $(DEST_DIR)/$(BASE_NAME).tes
TARGET_FFS_FILE   = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).pei

#
# Create our TE section from our TE file
#
$(TARGET_TES) : $(TARGET_TE)
	$(GENSECTION) -I $(TARGET_TE) -O $(TARGET_TES) -S EFI_SECTION_TE

#
# Run FWImage on the DLL to set it as an EFI image type.
#
$(TARGET_EFI) : $(TARGET_DLL) $(INF_FILENAME)
	$(FWIMAGE) $(COMPONENT_TYPE) $(TARGET_DLL) $(TARGET_EFI)

#
# Run GenTEImage on the built .efi file to create our TE file.
#
$(TARGET_TE) : $(TARGET_EFI) 
	$(GENTEIMAGE) -o $(TARGET_TE) $(TARGET_EFI)

#
# Link all objects and libs to create the executable
#
$(TARGET_DLL) : $(TARGET_LOCAL_OBJECT) $(LIBS) $(INF_FILENAME) $(ENV_DEPS)
	$(LINK) $(LINK_FLAGS_DLL) -e $(IMAGE_ENTRY_POINT) \
	-o $(TARGET_DLL) $(TARGET_LOCAL_OBJECT) $(LIBS)
	$(SETSTAMP) $(TARGET_DLL) $(BUILD_DIR)/GenStamp.txt
ifeq ($(EFI_GENERATE_SYM_FILE),YES)
  if exist $(TARGET_PDB) $(PE2SYM) $(TARGET_PDB) $(TARGET_SYM)
endif

ifeq ($(EFI_ZERO_DEBUG_DATA),YES)
	$(ZERODEBUGDATA) $(TARGET_DLL)
endif

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
ifneq ($(BUILD_NUMBER), )
ifndq ($(VERSION_STRING), )
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
else
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
endif
else
$(TARGET_VER) : 
	touch $(TARGET_VER)
endif

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
ifneq ($(DPX_SOURCE), )
DPX_SOURCE_FILE = $(SOURCE_DIR)/$(DPX_SOURCE)
endif

ifneq ($(DPX_SOURCE_OVERRIDE), )
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
endif

ifneq ($(DPX_SOURCE_FILE), )
ifneq ($(wildcard $(DPX_SOURCE_FILE)), )
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
	$(CC) $(INC) $(VERSION_FLAGS) -E -P -x c $(DPX_SOURCE_FILE) > $@.tmp1
	$(GENDEPEX) -I $@.tmp1 -O $@.tmp2
	$(GENSECTION) -I $@.tmp2 -O $@ -S $(DEPEX_TYPE)
	$(RM) $@.tmp1 > $(NULL)
	$(RM) $@.tmp2 > $(NULL)
else
$(error Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist. )
endif
else
$(TARGET_DPX) : 
	touch $(TARGET_DPX)
endif

#
# Build an FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_TES) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER) $(PACKAGE_FILENAME)
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

ifeq ($(CREATEDEPS),YES)
all : $(DEP_TARGETS)
	$(MAKE) -f $(MAKEFILE_NAME) all
else
all : $(TARGET_FFS_FILE) $(DEP_TARGETS)
endif

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
#
# These are the commands to build EBC EFI targets
#
[=============================================================================]
[Build.Ebc.BS_DRIVER|RT_DRIVER|APPLICATION]

#
# Add the EBC library to our list of libs
#
LIBS = $(LIBS) $(EBC_TOOLS_PATH)/lib/EbcLib.a 

ifeq ($(LOCALIZE),YES)

ifeq ($(EFI_GENERATE_HII_EXPORT),YES)
STRGATHER_FLAGS   += -hpk $(DEST_DIR)/$(BASE_NAME)Strings.hpk

#
# There will be one HII pack containing all the strings. Add that file
# to the list of HII pack files we'll use to create our final HII export file.
#
HII_PACK_FILES += $(DEST_DIR)/$(BASE_NAME)Strings.hpk

LOCALIZE_TARGETS  += $(DEST_DIR)/$(BASE_NAME).hii
endif

$(DEST_DIR)/$(BASE_NAME).sdb : $(SDB_FILES) $(SOURCE_FILES)
	$(STRGATHER) -scan -vdbr $(STRGATHER_FLAGS) -od $(DEST_DIR)/$(BASE_NAME).sdb \
    -skipext .uni -skipext .h $(SOURCE_FILES)

$(DEST_DIR)/$(BASE_NAME)Strings.c : $(DEST_DIR)/$(BASE_NAME).sdb
	$(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)/$(BASE_NAME).sdb \
    -oc $(DEST_DIR)/$(BASE_NAME)Strings.c

$(DEST_DIR)/$(BASE_NAME)StrDefs.h : $(DEST_DIR)/$(BASE_NAME).sdb
	$(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)/$(BASE_NAME).sdb \
    -oh $(DEST_DIR)/$(BASE_NAME)StrDefs.h

$(DEST_DIR)/$(BASE_NAME)Strings.hpk : $(DEST_DIR)/$(BASE_NAME).sdb
	$(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)/$(BASE_NAME).sdb \
    -hpk $(DEST_DIR)/$(BASE_NAME)Strings.hpk

OBJECTS += $(DEST_DIR)/$(BASE_NAME)Strings.o

$(DEST_DIR)/$(BASE_NAME)Strings.o : $(DEST_DIR)/$(BASE_NAME)Strings.c $(INF_FILENAME) $(ALL_DEPS)
	$(EBC_CC) $(EBC_C_FLAGS) $(DEST_DIR)/$(BASE_NAME)Strings.c

LOCALIZE_TARGETS += $(DEST_DIR)/$(BASE_NAME)StrDefs.h

endif

#
# If building an application, then the target is a .app, not .dxe
#
ifeq ($(COMPONENT_TYPE),APPLICATION)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).app
SUBSYSTEM       = EFI_APPLICATION
else
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).dxe
SUBSYSTEM       = EFI_BOOT_SERVICE_DRIVER
endif

#
# Defines for standard intermediate files and build targets
#
TARGET_EFI  = $(BIN_DIR)/$(BASE_NAME).efi
TARGET_DPX  = $(DEST_DIR)/$(BASE_NAME).dpx
TARGET_UI   = $(DEST_DIR)/$(BASE_NAME).ui
TARGET_VER  = $(DEST_DIR)/$(BASE_NAME).ver
TARGET_MAP  = $(BIN_DIR)/$(BASE_NAME).map
TARGET_PDB  = $(BIN_DIR)/$(BASE_NAME).pdb
TARGET_PE32 = $(DEST_DIR)/$(BASE_NAME).pe32
TARGET_DLL  = $(BIN_DIR)/$(BASE_NAME).elf

#
# First link all the objects and libs together to make a .elf file
#
$(TARGET_DLL) : $(OBJECTS) $(LIBS) $(INF_FILENAME) $(ENV_DEPS)
	$(EBC_LINK) $(EBC_LINK_FLAGS) /SUBSYSTEM:$(SUBSYSTEM) /ENTRY:EfiStart \
    $(OBJECTS) $(LIBS) /OUT:$(TARGET_DLL) /MAP:$(TARGET_MAP)
	$(SETSTAMP) $(TARGET_DLL) $(BUILD_DIR)/GenStamp.txt
ifeq ($(EFI_ZERO_DEBUG_DATA),YES)
	$(ZERODEBUGDATA) $(TARGET_DLL)
endif

#
# Now take the .elf file and make a .efi file
#
$(TARGET_EFI) : $(TARGET_DLL) $(INF_FILENAME)
	$(FWIMAGE) -t 0 $(COMPONENT_TYPE) $(TARGET_DLL) $(TARGET_EFI)

#
# Now take the .efi file and make a .pe32 section
#
$(TARGET_PE32) : $(TARGET_EFI) 
	$(GENSECTION) -I $(TARGET_EFI) -O $(TARGET_PE32) -S EFI_SECTION_PE32

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
ifneq ($(BUILD_NUMBER), )
ifneq ($(VERSION_STRING), )
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
else
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
endif
else
$(TARGET_VER) : 
	touch $(TARGET_VER)
endif

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
ifneq ($(DPX_SOURCE), )
DPX_SOURCE_FILE = $(SOURCE_DIR)/$(DPX_SOURCE)
endif

ifneq ($(DPX_SOURCE_OVERRIDE), )
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
endif

ifneq ($(DPX_SOURCE_FILE), )
ifneq ($(wildcard $(DPX_SOURCE_FILE)), )
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
	$(CC) $(INC) $(VERSION_FLAGS) -E -P -x c $(DPX_SOURCE_FILE) > $@.tmp1
	$(GENDEPEX) -I $@.tmp1 -O $@.tmp2
	$(GENSECTION) -I $@.tmp2 -O $@ -S $(DEPEX_TYPE)
	$(RM) $@.tmp1 > $(NULL)
	$(RM) $@.tmp2 > $(NULL)
else
$(error Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist. )
endif
else
$(TARGET_DPX) : 
	touch $(TARGET_DPX)
endif

#
# Describe how to build the HII export file from all the input HII pack files.
# Use the FFS file GUID for the package GUID in the export file. Only used
# when multiple VFR share strings.
#
$(DEST_DIR)/$(BASE_NAME).hii : $(HII_PACK_FILES)
	$(HIIPACK) create -g $(FILE_GUID) -p $(HII_PACK_FILES) -o $(DEST_DIR)/$(BASE_NAME).hii

#
# If the build calls for creating an FFS file with the IFR included as
# a separate binary (not compiled into the driver), then build the binary
# section now. Note that the PACKAGE must be set correctly to actually get
# this IFR section pulled into the FFS file.
#
ifneq ($(HII_IFR_PACK_FILES), )

$(DEST_DIR)/$(BASE_NAME)IfrBin.sec : $(HII_IFR_PACK_FILES)
	$(HIIPACK) create -novarpacks -p $(HII_IFR_PACK_FILES) -o $(DEST_DIR)/$(BASE_NAME)IfrBin.hii
	$(GENSECTION) -I $(DEST_DIR)/$(BASE_NAME)IfrBin.hii -O $(DEST_DIR)/$(BASE_NAME)IfrBin.sec -S EFI_SECTION_RAW

BIN_TARGETS = $(BIN_TARGETS) $(DEST_DIR)/$(BASE_NAME)IfrBin.sec

endif

#
# Build an FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_PE32) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER) $(PACKAGE_FILENAME)
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

ifeq ($(CREATEDEPS),YES)
all : $(DEP_TARGETS)
	$(MAKE) -f $(MAKEFILE_NAME) all
else
all : $(LOCALIZE_TARGETS) $(BIN_TARGETS) $(TARGET_FFS_FILE) $(DEP_TARGETS)
endif

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
#
# These are the commands to build vendor-provided *.EFI files into an FV.
# To use them, create an INF file with BUILD_TYPE=BS_DRIVER_EFI.
# This section, as it now exists, only supports boot service drivers.
#
[=============================================================================]
[Build.Mips32.BS_DRIVER_EFI|RT_DRIVER_EFI|APPLICATION_EFI,Build.Ipf.BS_DRIVER_EFI|RT_DRIVER_EFI|APPLICATION_EFI,Build.Ebc.BS_DRIVER_EFI|RT_DRIVER_EFI|APPLICATION_EFI,Build.x64.BS_DRIVER_EFI|RT_DRIVER_EFI|APPLICATION_EFI]
#
# Defines for standard intermediate files and build targets. For the source
# .efi file, take the one in the source directory if it exists. If there's not
# one there, look for one in the processor-specfic subdirectory.
#
ifneq ($(wildcard $(SOURCE_DIR)/$(BASE_NAME).efi), )
TARGET_EFI        = $(SOURCE_DIR)/$(BASE_NAME).efi
else ifneq ($(wildcard $(SOURCE_DIR)/$(PROCESSOR)/$(BASE_NAME).efi), )
TARGET_EFI        = $(SOURCE_DIR)/$(PROCESSOR)/$(BASE_NAME).efi
else
$(error Pre-existing $(BASE_NAME).efi file not found in $(SOURCE_DIR) nor $(SOURCE_DIR)/$(PROCESSOR) )
endif

TARGET_DPX        = $(DEST_DIR)/$(BASE_NAME).dpx
TARGET_UI         = $(DEST_DIR)/$(BASE_NAME).ui
TARGET_VER        = $(DEST_DIR)/$(BASE_NAME).ver
TARGET_MAP        = $(BIN_DIR)/$(BASE_NAME).map
TARGET_PDB        = $(BIN_DIR)/$(BASE_NAME).pdb
TARGET_PE32       = $(DEST_DIR)/$(BASE_NAME).pe32
TARGET_DLL        = $(BIN_DIR)/$(BASE_NAME).elf

#
# If building an application, then the target is a .app, not .dxe
#
ifeq ($(COMPONENT_TYPE),APPLICATION)
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).app
else
TARGET_FFS_FILE = $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).dxe
endif

#
# Take the .efi file and make a .pe32 file
#
$(TARGET_PE32) : $(TARGET_EFI) 
	$(GENSECTION) -I $(TARGET_EFI) -O $(TARGET_PE32) -S EFI_SECTION_PE32

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
ifneq ($(BUILD_NUMBER), )
ifneq ($(VERSION_STRING), )
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
else
$(TARGET_VER) : $(INF_FILENAME)
	$(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
endif
else
$(TARGET_VER) : 
	touch $(TARGET_VER)
endif

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
ifneq ($(DPX_SOURCE), )
DPX_SOURCE_FILE = $(SOURCE_DIR)/$(DPX_SOURCE)
endif

ifneq ($(DPX_SOURCE_OVERRIDE), )
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
endif

ifneq ($(DPX_SOURCE_FILE), )
ifneq ($(wildcard $(DPX_SOURCE_FILE)), )
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
	$(CC) $(INC) $(VERSION_FLAGS) -E -P -x c $(DPX_SOURCE_FILE) > $@.tmp1
	$(GENDEPEX) -I $@.tmp1 -O $@.tmp2
	$(GENSECTION) -I $@.tmp2 -O $@ -S $(DEPEX_TYPE)
	$(RM) $@.tmp1 > $(NULL)
	$(RM) $@.tmp2 > $(NULL)
else
$(error Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist. )
endif
else
$(TARGET_DPX) : 
	touch $(TARGET_DPX)
endif

#
# Build a FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_PE32) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER) $(PACKAGE_FILENAME)
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

all : $(TARGET_FFS_FILE)

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
[Compile.Mips32.Bin|Bmp,Compile.x64.Bin|Bmp,Compile.Ipf.Bin|Bmp]
#
# We simply copy the binary file from the source directory to the destination directory
#
$(DEST_DIR)/$(BASE_NAME).bin : $(SOURCE_FILE_NAME)
	$(CP) $< $@

[=============================================================================]
[Build.Mips32.BINARY|Legacy16|Logo,Build.Ipf.BINARY|Legacy16|Logo,Build.x64.BINARY|Legacy16|Logo]
#
# Use GenFfsFile to convert it to an FFS file
#
$(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).ffs : $(DEST_DIR)/$(BASE_NAME).bin $(PACKAGE_FILENAME)
	$(GENSECTION) -I $(DEST_DIR)/$(BASE_NAME).bin -O $(DEST_DIR)/$(BASE_NAME).sec -S EFI_SECTION_RAW
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

all : $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).ffs

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
[Build.Mips32.RAWFILE|CONFIG,Build.Ipf.RAWFILE|CONFIG,Build.x64.RAWFILE|CONFIG]
#
# Use GenFfsFile to convert it to an raw FFS file
#
$(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).raw : $(DEST_DIR)/$(BASE_NAME).bin $(PACKAGE_FILENAME)
	$(CD) $(OUT_DIR) &&	$(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

all : $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).raw

#
# Remove the generated temp and final files for this modules.
#
clean :
ifneq ($(FILE_GUID), )
	$(RM) $(BIN_DIR)/$(FILE_GUID)-$(BASE_NAME).*
endif
	$(RM) $(BIN_DIR)/$(BASE_NAME).*
	$(RMDIR) $(DEST_OUTPUT_DIRS) 

[=============================================================================]
# 
# These are commands to compile unicode .uni files.
#
[=============================================================================]
[Compile.Mips32.Uni,Compile.Ipf.Uni,Compile.Ebc.Uni,Compile.x64.Uni]
#
# Emit an error message if the file's base name is the same as the
# component base name. This causes build issues.
#
ifeq ($(FILE),$(BASE_NAME))
$(error Component Unicode string file name cannot be the same as the component $(BASE_NAME). )
endif

#
# Always create dep file for uni file as it can be created at the same time when 
# strgather is parsing uni file.
#
DEP_FILE    = $(DEST_DIR)/$(FILE)Uni.dep

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

$(DEST_DIR)/$(FILE).sdb : $(SOURCE_FILE_NAME) $(INF_FILENAME)
	$(STRGATHER) -parse -newdb -db $(DEST_DIR)/$(FILE).sdb -dep $(DEP_FILE) $(INC) $(SOURCE_FILE_NAME)

SDB_FILES       += $(DEST_DIR)/$(FILE).sdb
STRGATHER_FLAGS += -db $(DEST_DIR)/$(FILE).sdb
LOCALIZE        = YES

[=============================================================================]
[Compile.Mips32.Vfr,Compile.Ipf.Vfr,Compile.x64.Vfr]

DEP_FILE    = $(DEST_DIR)/$(FILE)Vfr.dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).o), )
DEP_TARGETS += $(DEST_DIR)/$(FILE)Vfr.dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).o
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS)

HII_PACK_FILES  += $(DEST_DIR)/$(FILE).hpk

#
# Add a dummy command for building the HII pack file. In reality, it's built 
# below, but the C_FLAGS macro reference the target as $@, so you can't specify
# the obj and hpk files as dual targets of the same command.
#
$(DEST_DIR)/$(FILE).hpk : $(DEST_DIR)/$(FILE).o
  
$(DEST_DIR)/$(FILE).o : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(VFRCOMPILE) $(VFRCOMPILE_FLAGS) $(INC) -ibin -od $(DEST_DIR)/$(SOURCE_RELATIVE_PATH) \
    -l $(VFR_FLAGS) $(SOURCE_FILE_NAME)
	$(CC) $(C_FLAGS) $(DEST_DIR)/$(FILE).c

[=============================================================================]
[Compile.Ebc.Vfr]

DEP_FILE    = $(DEST_DIR)/$(FILE)Vfr.dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).o), )
DEP_TARGETS += $(DEST_DIR)/$(FILE)Vfr.dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).o
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS)

HII_PACK_FILES  += $(DEST_DIR)/$(FILE).hpk

#
# Add a dummy command for building the HII pack file. In reality, it's built 
# below, but the C_FLAGS macro reference the target as $@, so you can't specify
# the obj and hpk files as dual targets of the same command.
#
$(DEST_DIR)/$(FILE).hpk : $(DEST_DIR)/$(FILE).o
  
$(DEST_DIR)/$(FILE).o : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(VFRCOMPILE) $(VFRCOMPILE_FLAGS) $(INC) -ibin -od $(DEST_DIR)/$(SOURCE_RELATIVE_PATH) \
    -l $(VFR_FLAGS) $(SOURCE_FILE_NAME)
	$(EBC_CC) $(EBC_C_FLAGS) $(DEST_DIR)/$(FILE).c

[=============================================================================]
#
# Commands for building IFR as uncompressed binary into the FFS file. To 
# use it, set COMPILE_SELECT=.vfr=Ifr_Bin for the component in the DSC file.
#
[=============================================================================]
[Compile.Mips32.Ifr_Bin,Compile.Ipf.Ifr_Bin,Compile.x64.Ifr_Bin]

DEP_FILE    = $(DEST_DIR)/$(FILE)Vfr.dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).o), )
DEP_TARGETS += $(DEST_DIR)/$(FILE)Vfr.dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).o
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS)

HII_PACK_FILES  += $(DEST_DIR)/$(FILE).hpk

#
# Add a dummy command for building the HII pack file. In reality, it's built 
# below, but the C_FLAGS macro reference the target as $@, so you can't specify
# the obj and hpk files as dual targets of the same command.
#
$(DEST_DIR)/$(FILE).hpk : $(DEST_DIR)/$(FILE).o
  
$(DEST_DIR)/$(FILE).o : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(VFRCOMPILE) $(VFRCOMPILE_FLAGS) $(INC) -ibin -od $(DEST_DIR)/$(SOURCE_RELATIVE_PATH) \
    -l $(VFR_FLAGS) $(SOURCE_FILE_NAME)
	$(CC) $(C_FLAGS) $(DEST_DIR)/$(FILE).c

#
# Add to the variable that contains the list of VFR binary files we're going
# to merge together at the end of the build. 
#
HII_IFR_PACK_FILES = $(HII_IFR_PACK_FILES) $(DEST_DIR)/$(FILE).hpk

[=============================================================================]
#
# Commands for building IFR as uncompressed binary into the FFS file. To 
# use it, set COMPILE_SELECT=.vfr=Ifr_Bin for the component in the DSC file.
#
[=============================================================================]
[Compile.Ebc.Ifr_Bin]

DEP_FILE    = $(DEST_DIR)/$(FILE)Vfr.dep

ifneq ($(wildcard $(DEST_DIR)/$(FILE).o), )
DEP_TARGETS += $(DEST_DIR)/$(FILE)Vfr.dep
ifeq ($(wildcard $(DEP_FILE)), )
CREATEDEPS = YES
endif
endif

ifneq ($(wildcard $(DEP_FILE)), )
include $(DEP_FILE)
endif

#
# Update dep file for next round incremental build
#
$(DEP_FILE) : $(DEST_DIR)/$(FILE).o
	$(MAKEDEPS) -f $(SOURCE_FILE_NAME) $(DEP_FLAGS)

HII_PACK_FILES  += $(DEST_DIR)/$(FILE).hpk

#
# Add a dummy command for building the HII pack file. In reality, it's built 
# below, but the C_FLAGS macro reference the target as $@, so you can't specify
# the obj and hpk files as dual targets of the same command.
#
$(DEST_DIR)/$(FILE).hpk : $(DEST_DIR)/$(FILE).o
  
$(DEST_DIR)/$(FILE).o : $(SOURCE_FILE_NAME) $(INF_FILENAME) $(ALL_DEPS)
	$(VFRCOMPILE) $(VFRCOMPILE_FLAGS) $(INC) -ibin -od $(DEST_DIR)/$(SOURCE_RELATIVE_PATH) \
    -l $(VFR_FLAGS) $(SOURCE_FILE_NAME)
	$(EBC_CC) $(EBC_C_FLAGS) $(DEST_DIR)/$(FILE).c

#
# Add to the variable that contains the list of VFR binary files we're going
# to merge together at the end of the build. 
#
HII_IFR_PACK_FILES = $(HII_IFR_PACK_FILES) $(DEST_DIR)/$(FILE).hpk

[=============================================================================]
[Compile.Mips32.Fv,Compile.Ipf.Fv,Compile.x64.Fv]
#
# Run GenSection on the firmware volume image.
#
$(DEST_DIR)/$(SOURCE_FV)Fv.sec : $(SOURCE_FILE_NAME)
	$(GENSECTION) -I $(SOURCE_FILE_NAME) -O $(DEST_DIR)/$(SOURCE_FV)Fv.sec -S EFI_SECTION_FIRMWARE_VOLUME_IMAGE

[=============================================================================]
