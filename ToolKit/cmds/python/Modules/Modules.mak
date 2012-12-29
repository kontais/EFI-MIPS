#
# Copyright (c) 1999, 2000
# Intel Corporation.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. All advertising materials mentioning features or use of this software must
#    display the following acknowledgement:
# 
#    This product includes software developed by Intel Corporation and its
#    contributors.
# 
# 4. Neither the name of Intel Corporation or its contributors may be used to
#    endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

#
# Include sdk.env environment
#

include $(SDK_INSTALL_DIR)/build/$(SDK_BUILD_ENV)/sdk.env

#
# Defining DYNAMIC_PYTHON_MODULES causes many of the lesser used
#  modules to be built as EFI drivers that are loaded on demand
#  rather than being statically linked into the executable.
# Note:
# We dont' support dnyamic modules from porting Python 2.4 into EFI Toolkit
#

#DYNAMIC_PYTHON_MODULES = 0

#
# Set the base output name
#

BASE_NAME = Modules

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/cmds/python/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/cmds/python/$(BASE_NAME)

LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libc/libc.a \
       $(SDK_INSTALL_DIR)/lib/libefi/libefi.a \
       $(SDK_INSTALL_DIR)/lib/libm/libm.a \
       $(SDK_INSTALL_DIR)/lib/libdb/libdb.a \
       $(SDK_INSTALL_DIR)/lib/libz/libz.a \
       $(SDK_INSTALL_DIR)/lib/libsocket/libsocket.a \
       $(BUILD_DIR)/EfiInterfaceLib/EfiInterfaceLib.a

#
# Rule for how to build a dynamically loaded module
#

{$(BUILD_DIR)}.o{$(BIN_DIR)}.pym:

%.pym:%.o
	$(LINK) $(LINK_FLAGS_EXE) $(MODULE_LFLAGS) -e EfiPythonModuleEntry -o $@.exe
	$(LINK) $(LINK_FLAGS_DLL) $(MODULE_LFLAGS) -e EfiPythonModuleEntry -o $@.dll
	$(FWIMAGE) bsdrv $@.dll $@

#
# Additional compile flags
#

#C_FLAGS += -D __STDC__ 
ifdef DYNAMIC_PYTHON_MODULES
C_FLAGS = /D EFI_LOADABLE_MODULE $(C_FLAGS)
endif

#
# The dynamically loaded modules
#

ifdef DYNAMIC_PYTHON_MODULES
DYNAMIC_MODULES = $(BIN_DIR)/arraymodule.pym \
		  $(BIN_DIR)/bsddbmodule.pym \
          $(BIN_DIR)/cmathmodule.pym \
		  $(BIN_DIR)/cpickle.pym \
		  $(BIN_DIR)/cstringio.pym \
		  $(BIN_DIR)/dbmmodule.pym \
		  $(BIN_DIR)/errnomodule.pym \
		  $(BIN_DIR)/mathmodule.pym \
		  $(BIN_DIR)/md5module.pym  \
		  $(BIN_DIR)/operator.pym \
		  $(BIN_DIR)/parsermodule.pym \
		  $(BIN_DIR)/regexmodule.pym \
		  $(BIN_DIR)/selectmodule.pym \
		  $(BIN_DIR)/shamodule.pym \
		  $(BIN_DIR)/socketmodule.pym \
		  $(BIN_DIR)/structmodule.pym \
		  $(BIN_DIR)/timemodule.pym \
		  $(BIN_DIR)/timingmodule.pym \
		  $(BIN_DIR)/zlibmodule.pym 
endif

#
# Default target
#

all : $(OBJECTS) $(DYNAMIC_MODULES)

#
# Dynamic module dependencies
#

ifdef DYNAMIC_PYTHON_MODULES

$(BIN_DIR)/arraymodule.pym  : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/bsddbmodule.pym  : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/cmathmodule.pym  : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/cpickle.pym      : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/cstringio.pym    : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/dbmmodule.pym    : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/errnomodule.pym  : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/mathmodule.pym   : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/md5module.pym    : $(BUILD_DIR)/$(*B).o $(BUILD_DIR)/md5c.o $(LIBS)
$(BIN_DIR)/operator.pym     : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/parsermodule.pym : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/regexmodule.pym  : $(BUILD_DIR)/$(*B).o $(BUILD_DIR)/regexpr.o $(LIBS)
$(BIN_DIR)/selectmodule.pym : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/shamodule.pym    : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/socketmodule.pym : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/structmodule.pym : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/timemodule.pym   : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/timingmodule.pym : $(BUILD_DIR)/$(*B).o $(LIBS)
$(BIN_DIR)/zlibmodule.pym   : $(BUILD_DIR)/$(*B).o $(LIBS)

endif

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

include $(SDK_INSTALL_DIR)/include/efishell/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/efishell

include ../include/makefile.hdr
INC += -I ../include

#
# Local include dependencies
#

INC_DEPS +=  \
	md5.h \
	regexpr.h \
	timing.h \


#
# Local libraries dependencies
#

$(BUILD_DIR)/EfiInterfaceLib/EfiInterfaceLib.a : 
	cd EfiInterfaceLib
	nmake -f EfiInterfaceLib.mak all
	cd $(SOURCE_DIR)

#
# Library object files
#

ifndef DYNAMIC_PYTHON_MODULES

OBJECTS +=  \
    $(BUILD_DIR)/_codecsmodule.o \
    $(BUILD_DIR)/_randommodule.o \
    $(BUILD_DIR)/_sre.o \
    $(BUILD_DIR)/binascii.o \
    $(BUILD_DIR)/bsddbmodule.o \
    $(BUILD_DIR)/config.o \
    $(BUILD_DIR)/getpath.o \
    $(BUILD_DIR)/main.o \
    $(BUILD_DIR)/getbuildinfo.o \
    $(BUILD_DIR)/python.o \
    $(BUILD_DIR)/arraymodule.o \
    $(BUILD_DIR)/cmathmodule.o \
    $(BUILD_DIR)/efimodule.o \
    $(BUILD_DIR)/errnomodule.o \
    $(BUILD_DIR)/mathmodule.o \
    $(BUILD_DIR)/md5module.o \
    $(BUILD_DIR)/md5c.o \
    $(BUILD_DIR)/operator.o \
    $(BUILD_DIR)/parsermodule.o \
    $(BUILD_DIR)/regexmodule.o \
    $(BUILD_DIR)/regexpr.o \
    $(BUILD_DIR)/selectmodule.o \
    $(BUILD_DIR)/shamodule.o \
    $(BUILD_DIR)/socketmodule.o \
    $(BUILD_DIR)/stropmodule.o \
    $(BUILD_DIR)/structmodule.o \
    $(BUILD_DIR)/timemodule.o \
    $(BUILD_DIR)/timingmodule.o \
    $(BUILD_DIR)/cstringio.o \
    $(BUILD_DIR)/cpickle.o \
    $(BUILD_DIR)/dbmmodule.o \
    $(BUILD_DIR)/zlibmodule.o \
    $(BUILD_DIR)/gcmodule.o \
    $(BUILD_DIR)/unicodedata.o \
    $(BUILD_DIR)/itertoolsmodule.o \

else

OBJECTS +=  \
    $(BUILD_DIR)/config.o \
    $(BUILD_DIR)/getpath.o \
    $(BUILD_DIR)/main.o \
    $(BUILD_DIR)/getbuildinfo.o \
    $(BUILD_DIR)/python.o \
    $(BUILD_DIR)/efimodule.o \
    $(BUILD_DIR)/stropmodule.o \
    $(BUILD_DIR)/gcmodule.o \

endif

#
# Source file dependencies
#
$(BUILD_DIR)/_codecsmodule.o:  $(INC_DEPS)
$(BUILD_DIR)/_randommodule.o:  $(INC_DEPS)
$(BUILD_DIR)/_sre.o         :  $(INC_DEPS)
$(BUILD_DIR)/binascii.o     :  $(INC_DEPS)
$(BUILD_DIR)/bsddbmodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/config.o       :  $(INC_DEPS)
$(BUILD_DIR)/getpath.o      :  $(INC_DEPS)
$(BUILD_DIR)/main.o         :  $(INC_DEPS)
$(BUILD_DIR)/getbuildinfo.o :  $(INC_DEPS)
$(BUILD_DIR)/python.o       :  $(INC_DEPS)
$(BUILD_DIR)/arraymodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/cmathmodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/efimodule.o    :  $(INC_DEPS)
$(BUILD_DIR)/errnomodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/mathmodule.o   :  $(INC_DEPS)
$(BUILD_DIR)/md5module.o    :  $(INC_DEPS)
$(BUILD_DIR)/md5c.o         :  $(INC_DEPS)
$(BUILD_DIR)/operator.o     :  $(INC_DEPS)
$(BUILD_DIR)/parsermodule.o :  $(INC_DEPS)
$(BUILD_DIR)/regexmodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/regexpr.o      :  $(INC_DEPS)
$(BUILD_DIR)/selectmodule.o :  $(INC_DEPS)
$(BUILD_DIR)/shamodule.o    :  $(INC_DEPS)
$(BUILD_DIR)/socketmodule.o :  $(INC_DEPS)
$(BUILD_DIR)/stropmodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/structmodule.o :  $(INC_DEPS)
$(BUILD_DIR)/timemodule.o   :  $(INC_DEPS)
$(BUILD_DIR)/timingmodule.o :  $(INC_DEPS)
$(BUILD_DIR)/cstringio.o    :  $(INC_DEPS)
$(BUILD_DIR)/cpickle.o      :  $(INC_DEPS)
$(BUILD_DIR)/bsddbmodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/dbmmodule.o    :  $(INC_DEPS)
$(BUILD_DIR)/zlibmodule.o   :  $(INC_DEPS)
$(BUILD_DIR)/gcmodule.o     :  $(INC_DEPS)
$(BUILD_DIR)/unicodedata.o     :  $(INC_DEPS)
$(BUILD_DIR)/itertoolsmodule.o :  $(INC_DEPS)

#
# Because the target of this makefile is a library, we can't use the
# library targets in master.env
#

$(SDK_INSTALL_DIR)/lib/libc/libc.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libc
	nmake -f libc.mak all
	cd $(SOURCE_DIR)

$(SDK_INSTALL_DIR)/lib/libsocket/libsocket.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libsocket
	nmake -f libsocket.mak all
	cd $(SOURCE_DIR)

$(SDK_INSTALL_DIR)/lib/libm/libm.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libm
	nmake -f libm.mak all
	cd $(SOURCE_DIR)

$(SDK_INSTALL_DIR)/lib/libdb/libdb.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libdb
	nmake -f libdb.mak all
	cd $(SOURCE_DIR)

$(SDK_INSTALL_DIR)/lib/libz/libz.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libz
	nmake -f libz.mak all
	cd $(SOURCE_DIR)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
