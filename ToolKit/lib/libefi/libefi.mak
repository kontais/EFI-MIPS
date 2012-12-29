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
#  Set the base output name
#

BASE_NAME = libefi

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
#BUILD_DIR  = $(SDK_BUILD_DIR)/lib/$(BASE_NAME)

#
# Additional compile flags
#


#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/efishell/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/efishell \
      -I $(SDK_INSTALL_DIR)/include/efishell/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/protocol/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/protocol \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/protocol/$(PROCESSOR)

include ./makefile.hdr
INC += -I . \
      -I ./$(PROCESSOR)

#
# Default target
#

#all : $(OBJECTS)
all : $(OBJECTS)

#
# Local sub directories
#

sub_dirs : $(BUILD_DIR)/Runtime \
           $(BUILD_DIR)/$(PROCESSOR) \
	
#
# Directory targets
#

$(BUILD_DIR)/Runtime : ; - md $(BUILD_DIR)/Runtime

#
#  Library object files
#

OBJECTS += \
    $(BUILD_DIR)/BoxDraw.o \
    $(BUILD_DIR)/console.o \
    $(BUILD_DIR)/crc.o \
    $(BUILD_DIR)/data.o \
    $(BUILD_DIR)/debug.o \
    $(BUILD_DIR)/dpath.o \
    $(BUILD_DIR)/error.o \
    $(BUILD_DIR)/event.o \
    $(BUILD_DIR)/guid.o \
    $(BUILD_DIR)/hand.o \
    $(BUILD_DIR)/init.o \
    $(BUILD_DIR)/lock.o \
    $(BUILD_DIR)/misc.o \
    $(BUILD_DIR)/print.o \
    $(BUILD_DIR)/sread.o \
    $(BUILD_DIR)/str.o \
    $(BUILD_DIR)/hw.o \
    $(BUILD_DIR)/Smbios.o \
    $(BUILD_DIR)/systable.o\
\
    $(BUILD_DIR)/Runtime/EfiRtLib.o \
    $(BUILD_DIR)/Runtime/lock.o \
    $(BUILD_DIR)/Runtime/rtdata.o \
    $(BUILD_DIR)/Runtime/str.o \
    $(BUILD_DIR)/Runtime/vm.o \

#
#  Processor dependent targets
#

ifeq ($(PROCESSOR),Mips32)
OBJECTS +=  \
    $(BUILD_DIR)/$(PROCESSOR)/initplat.o \
    $(BUILD_DIR)/$(PROCESSOR)/math.o
endif

#
# Source file dependencies
#

$(BUILD_DIR)/BoxDraw.o          : BoxDraw.c  $(INC_DEPS)
$(BUILD_DIR)/console.o          : console.c  $(INC_DEPS)
$(BUILD_DIR)/crc.o              : crc.c      $(INC_DEPS)
$(BUILD_DIR)/data.o             : data.c     $(INC_DEPS)
$(BUILD_DIR)/debug.o            : debug.c    $(INC_DEPS)
$(BUILD_DIR)/dpath.o            : dpath.c    $(INC_DEPS)
$(BUILD_DIR)/error.o            : error.c    $(INC_DEPS)
$(BUILD_DIR)/event.o            : event.c    $(INC_DEPS)
$(BUILD_DIR)/guid.o             : guid.c     $(INC_DEPS)
$(BUILD_DIR)/hand.o             : hand.c     $(INC_DEPS)
$(BUILD_DIR)/init.o             : init.c     $(INC_DEPS)
$(BUILD_DIR)/lock.o             : lock.c     $(INC_DEPS)
$(BUILD_DIR)/misc.o             : misc.c     $(INC_DEPS)
$(BUILD_DIR)/print.o            : print.c    $(INC_DEPS)
$(BUILD_DIR)/sread.o            : sread.c    $(INC_DEPS)
$(BUILD_DIR)/str.o              : str.c      $(INC_DEPS)
$(BUILD_DIR)/hw.o               : hw.c       $(INC_DEPS)
$(BUILD_DIR)/Smbios.o           : Smbios.c   $(INC_DEPS)
$(BUILD_DIR)/systable.o         : systable.c $(INC_DEPS)

$(BUILD_DIR)/Runtime/EfiRtLib.o : Runtime/EfiRtLib.c $(INC_DEPS)
$(BUILD_DIR)/Runtime/lock.o     : Runtime/lock.c     $(INC_DEPS)
$(BUILD_DIR)/Runtime/rtdata.o   : Runtime/rtdata.c   $(INC_DEPS)
$(BUILD_DIR)/Runtime/str.o      : Runtime/str.c      $(INC_DEPS)
$(BUILD_DIR)/Runtime/vm.o       : Runtime/vm.c       $(INC_DEPS)

#
#  Processor dependent targets
#

$(BUILD_DIR)/$(PROCESSOR)/initplat.o : $(PROCESSOR)/initplat.c $(INC_DEPS)
$(BUILD_DIR)/$(PROCESSOR)/math.o     : $(PROCESSOR)/math.s     $(INC_DEPS)


#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
