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
# Set the base output name
#

BASE_NAME = Objects

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/cmds/python/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/cmds/python/$(BASE_NAME)

#
# Additional compile flags
#

#C_FLAGS += -D __STDC__

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

include ../include/makefile.hdr
INC += -I ../include

#
# Default target
#

all : $(OBJECTS)

#
# Local include dependencies
#

#
# Library object files
#

OBJECTS +=  \
    $(BUILD_DIR)/abstract.o \
    $(BUILD_DIR)/boolobject.o \
    $(BUILD_DIR)/bufferobject.o \
    $(BUILD_DIR)/cellobject.o \
    $(BUILD_DIR)/classobject.o \
    $(BUILD_DIR)/cobject.o \
    $(BUILD_DIR)/complexobject.o \
    $(BUILD_DIR)/descrobject.o \
    $(BUILD_DIR)/dictobject.o \
    $(BUILD_DIR)/enumobject.o \
    $(BUILD_DIR)/fileobject.o \
    $(BUILD_DIR)/floatobject.o \
    $(BUILD_DIR)/frameobject.o \
    $(BUILD_DIR)/funcobject.o \
    $(BUILD_DIR)/genobject.o \
    $(BUILD_DIR)/intobject.o \
    $(BUILD_DIR)/iterobject.o \
    $(BUILD_DIR)/listobject.o \
    $(BUILD_DIR)/longobject.o \
    $(BUILD_DIR)/methodobject.o \
    $(BUILD_DIR)/moduleobject.o \
    $(BUILD_DIR)/object.o \
    $(BUILD_DIR)/obmalloc.o \
    $(BUILD_DIR)/rangeobject.o \
    $(BUILD_DIR)/setobject.o \
    $(BUILD_DIR)/sliceobject.o \
    $(BUILD_DIR)/stringobject.o \
    $(BUILD_DIR)/structseq.o \
    $(BUILD_DIR)/tupleobject.o \
    $(BUILD_DIR)/typeobject.o \
    $(BUILD_DIR)/unicodectype.o \
    $(BUILD_DIR)/unicodeobject.o \
    $(BUILD_DIR)/weakrefobject.o \
        

#
# Source file dependencies
#


#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
