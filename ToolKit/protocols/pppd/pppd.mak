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

BASE_NAME = pppd

#
# Set entry point
#

IMAGE_ENTRY_POINT = EfiHook

#
# Globals needed by master.mak
#

TARGET_BS_DRIVER = $(BASE_NAME)
SOURCE_DIR       = $(SDK_INSTALL_DIR)/protocols/$(BASE_NAME)
BUILD_DIR        = $(SDK_INSTALL_DIR)/protocols/$(BASE_NAME)

#
# Additional compiler flags
#

LIBMD_SUPPORT = NO

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
       $(SDK_INSTALL_DIR)/lib/libsocket/libsocket.a \
       $(SDK_INSTALL_DIR)/lib/libtty/libtty.a \
       $(SDK_INSTALL_DIR)/lib/libc/libc.a
       
ifeq ($(LIBMD_SUPPORT),YES)
LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libmd/libmd.a \
       $(SDK_INSTALL_DIR)/lib/libcrypt/libcrypt.a 
endif

#
# Additional compile flags
#

C_FLAGS += -D__FreeBSD__ -DNO_DRAND48

#
# Adjust flags for libmd support
#
ifeq ($(LIBMD_SUPPORT),YES)
C_FLAGS += -DLIBMD_SUPPORT
endif

#
# Main targets
#

all : dirs $(LIBS) $(OBJECTS)

#
# Program object files
#

OBJECTS +=  \
    $(BUILD_DIR)/main.o \
    $(BUILD_DIR)/sys-bsd.o \
    $(BUILD_DIR)/fsm.o \
    $(BUILD_DIR)/ipcp.o \
    $(BUILD_DIR)/lcp.o \
    $(BUILD_DIR)/magic.o \
    $(BUILD_DIR)/cbcp.o \
    $(BUILD_DIR)/ccp.o \
    $(BUILD_DIR)/upap.o \
    $(BUILD_DIR)/options.o \
    $(BUILD_DIR)/auth.o \
    $(BUILD_DIR)/stubs.o \
    $(BUILD_DIR)/EfiHook.o 

ifeq ($(LIBMD_SUPPORT),YES)
OBJECTS +=  \
    $(BUILD_DIR)/chap.o
endif


#
# Source file dependencies
#

$(BUILD_DIR)/main.o    :  $(INC_DEPS)
$(BUILD_DIR)/sys-bsd.o :  $(INC_DEPS)
$(BUILD_DIR)/fsm.o     :  $(INC_DEPS)
$(BUILD_DIR)/ipcp.o    :  $(INC_DEPS)
$(BUILD_DIR)/lcp.o     :  $(INC_DEPS)
$(BUILD_DIR)/magic.o   :  $(INC_DEPS)
$(BUILD_DIR)/auth.o    :  $(INC_DEPS)
$(BUILD_DIR)/cbcp.o    :  $(INC_DEPS)
$(BUILD_DIR)/ccp.o     :  $(INC_DEPS)
$(BUILD_DIR)/chap.o    :  $(INC_DEPS)
$(BUILD_DIR)/upap.o    :  $(INC_DEPS)
$(BUILD_DIR)/options.o :  $(INC_DEPS)
$(BUILD_DIR)/stubs.o   :  $(INC_DEPS)
$(BUILD_DIR)/EfiHook.o :  $(INC_DEPS)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
