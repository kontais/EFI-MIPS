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
# Set the base output name and type for this makefile
#

BASE_NAME = ed

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

SOURCE_DIR = $(SDK_INSTALL_DIR)/cmds/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/cmds/$(BASE_NAME)
TARGET_APP = $(BASE_NAME)

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

#
# Libraries
#

LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libc/libc.a 
ifdef OLD_SHELL
       $(SDK_INSTALL_DIR)/lib/libefi/libefi.a \
       $(SDK_INSTALL_DIR)/lib/libefishell/libefishell.a 
endif

#
#  Default target
#

all : dirs $(LIBS) $(OBJECTS)

#
#  Local include dependencies
#

INC_DEPS +=  \
	ed.h

#
#  Application object files
#

OBJECTS +=  \
    $(BUILD_DIR)/buf.o \
    $(BUILD_DIR)/cbc.o \
    $(BUILD_DIR)/glbl.o \
    $(BUILD_DIR)/io.o \
    $(BUILD_DIR)/main.o \
    $(BUILD_DIR)/re.o \
    $(BUILD_DIR)/sub.o \
    $(BUILD_DIR)/undo.o

#
# Source file dependencies
#

$(BUILD_DIR)/buf.o  :  $(INC_DEPS)
$(BUILD_DIR)/cbc.o  :  $(INC_DEPS)
$(BUILD_DIR)/glbl.o :  $(INC_DEPS)
$(BUILD_DIR)/io.o   :  $(INC_DEPS)
$(BUILD_DIR)/main.o :  $(INC_DEPS)
$(BUILD_DIR)/re.o   :  $(INC_DEPS)
$(BUILD_DIR)/sub.o  :  $(INC_DEPS)
$(BUILD_DIR)/undo.o :  $(INC_DEPS)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
