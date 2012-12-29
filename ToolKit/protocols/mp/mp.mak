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

BASE_NAME = mp 

#
# Set entry point
#

IMAGE_ENTRY_POINT = MpProtocolEntry

#
# Globals needed by master.mak
#

TARGET_BS_DRIVER = $(BASE_NAME)
SOURCE_DIR       = $(SDK_INSTALL_DIR)/protocols/$(BASE_NAME)
BUILD_DIR        = $(SDK_INSTALL_DIR)/protocols/$(BASE_NAME)/$(PROCESSOR)

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

include makefile.hdr
INC += -I .

#
# Libraries
#

ifeq ($(PROCESSOR),Ia64)
LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libefi/libefi.a \
       $(SDK_INSTALL_DIR)/lib/libc/libc.a \
       $(SDK_INSTALL_DIR)/lib/libefishell/libefishell.a
endif
ifeq ($(PROCESSOR),Em64t)
LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libefi/libefi.a \
       $(SDK_INSTALL_DIR)/lib/libc/libc.a
endif

#
# Main targets
#

all : dirs $(LIBS) $(OBJECTS)


#
# Program object files
#

OBJECTS +=  
ifeq ($(PROCESSOR),Ia64)
    $(BUILD_DIR)/acpi.o \
    $(BUILD_DIR)/api_deinitpro.o \
    $(BUILD_DIR)/api_infoproc.o \
    $(BUILD_DIR)/api_initpro.o \
    $(BUILD_DIR)/api_numproc.o \
    $(BUILD_DIR)/api_startproc.o \
    $(BUILD_DIR)/api_startprocadd.o \
    $(BUILD_DIR)/api_stopproc.o \
    $(BUILD_DIR)/comm.o \
    $(BUILD_DIR)/entry.o \
    $(BUILD_DIR)/interface.o \
    $(BUILD_DIR)/proclist.o \
    $(BUILD_DIR)/stopallproc.o \
    $(BUILD_DIR)/util.o \
    $(BUILD_DIR)/wakeup.o \
    $(BUILD_DIR)/bootrendez.o \
    $(BUILD_DIR)/chain.o \
    $(BUILD_DIR)/misc.o 
endif

ifeq ($(PROCESSOR),Em64t)
    $(BUILD_DIR)/mpx64asm.o 
endif

#
# Source file dependencies
#

ifeq ($(PROCESSOR),Ia64)
$(BUILD_DIR)/acpi.o         		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/api_deinitpro.o	 	: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/api_infoproc.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/api_initpro.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/api_numproc.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/api_startproc.o	 	: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/api_startprocadd.o 	: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/api_stopproc.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/comm.o 			: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/entry.o 			: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/interface.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/proclist.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/stopallproc.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/util.o 			: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/wakeup.o 		: $(PROCESSOR)/ $(INC_DEPS)
$(BUILD_DIR)/bootrendez.o 		: $(PROCESSOR)/$(*B).s $(INC_DEPS)
$(BUILD_DIR)/chain.o 			: $(PROCESSOR)/$(*B).s $(INC_DEPS)
$(BUILD_DIR)/misc.o 			: $(PROCESSOR)/$(*B).s $(INC_DEPS)
endif

ifeq ($(PROCESSOR),Em64t)
$(BUILD_DIR)/mpx64asm.o     : $(PROCESSOR)/$(*B).s $(INC_DEPS)
    $(ASM) $(AFLAGS) $(MODULE_AFLAGS) /Fo$@ $(PROCESSOR)/$(*B).s
endif

#
# Handoff to Master.Mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
