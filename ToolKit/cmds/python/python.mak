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
# include sdk.env environment
#

include $(SDK_INSTALL_DIR)/build/$(SDK_BUILD_ENV)/sdk.env

#
# Set the base output name
#

BASE_NAME = python

#
# Set entry point
#

ifdef OLD_SHELL
IMAGE_ENTRY_POINT = _LIBC_Start_Shellapp_A
else
IMAGE_ENTRY_POINT = _LIBC_Start_A
endif

#
# Globals needed by master.mak
#

TARGET_APP = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/cmds/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/cmds/$(BASE_NAME)

#
# Additional compile flags
#

#C_FLAGS += -D __STDC__

#
# include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

include include/makefile.hdr
INC += -I include

#
# Local libraries
#

LIBS += $(BUILD_DIR)/Parser/Parser.a \
	   $(BUILD_DIR)/Objects/Objects.a \
	   $(BUILD_DIR)/Python/Python.a \
	   $(BUILD_DIR)/Modules/Modules.a

#
# Toolkit Libraries
#

ifdef OLD_SHELL
LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libefishell/libefishell.a
endif

LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libm/libm.a \
       $(SDK_INSTALL_DIR)/lib/libsocket/libsocket.a \
       $(SDK_INSTALL_DIR)/lib/libdb/libdb.a \
       $(SDK_INSTALL_DIR)/lib/libz/libz.a \
       $(SDK_INSTALL_DIR)/lib/libefi/libefi.a \
       $(SDK_INSTALL_DIR)/lib/libtty/libtty.a \
       $(SDK_INSTALL_DIR)/lib/libc/libc.a

all : dirs $(LIBS) $(OBJECTS)

#
# Local libraries dependencies
#

$(BUILD_DIR)/Modules/Modules.a : 
	cd Modules && $(MAKE) -f Modules.mak all

$(BUILD_DIR)/Objects/Objects.a :
	cd Objects && $(MAKE) -f Objects.mak all

$(BUILD_DIR)/Parser/Parser.a :
	cd Parser && $(MAKE) -f Parser.mak all

$(BUILD_DIR)/Python/Python.a :
	cd Python && $(MAKE) -f Python.mak all

#
#  Program object file 
#

OBJECTS +=  \
    $(BUILD_DIR)/EfiMain.o 

#
# Source file dependencies
#

$(BUILD_DIR)/EfiMain.o :  $(INC_DEPS)

cleanall :
	cd Modules && $(MAKE) -f Modules.mak clean
	cd Objects && $(MAKE) -f Objects.mak clean
	cd Parser && $(MAKE) -f Parser.mak clean 
	cd Python && $(MAKE) -f Python.mak clean

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
