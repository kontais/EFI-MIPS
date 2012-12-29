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

include Make.defaults

#
# Set the base output name and entry point
#

BASE_NAME         = elilo
IMAGE_ENTRY_POINT = efi_main

#
# Globals needed by master.mak
#

TARGET_APP = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/apps/elilo
BUILD_DIR  = $(SDK_INSTALL_DIR)/apps/$(BASE_NAME)

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

INC += -I $(SOURCE_DIR) \
       -I $(SOURCE_DIR)/$(EFI_INC_DIR)

#
# Libraries
#

LIBS +=  $(SDK_INSTALL_DIR)/lib/libefi/libefi.a



FILESYSTEM    =

ifeq ($(CONFIG_localfs),y) 
FILESYSTEMS += glue_localfs.o 
endif

ifeq ($(CONFIG_ext2fs),y) 
FILESYSTEMS += glue_ext2fs.o 
endif

ifeq ($(CONFIG_netfs),y) 
FILESYSTEMS += glue_netfs.o 
endif

#
# Default target
#

all : dirs $(LIBS) $(OBJECTS)

#
# Program object files
#

OBJECTS +=  \
	$(BUILD_DIR)/elilo.o \
	$(BUILD_DIR)/getopt.o \
	$(BUILD_DIR)/strops.o \
	$(BUILD_DIR)/loader.o \
	$(BUILD_DIR)/fileops.o \
	$(BUILD_DIR)/util.o \
	$(BUILD_DIR)/vars.o \
	$(BUILD_DIR)/alloc.o \
	$(BUILD_DIR)/chooser.o \
	$(BUILD_DIR)/config.o \
	$(BUILD_DIR)/initrd.o \
	$(BUILD_DIR)/alternate.o \
	$(BUILD_DIR)/bootparams.o \
	$(BUILD_DIR)/gunzip.o \
	$(BUILD_DIR)/console.o

ifeq ($(CONFIG_localfs),y)
OBJECTS += $(BUILD_DIR)/fs/localfs.o
endif

ifeq ($(CONFIG_ext2fs),y)
OBJECTS += $(BUILD_DIR)/fs/ext2fs.o
endif

ifeq ($(CONFIG_netfs),y)
OBJECTS += $(BUILD_DIR)/fs/netfs.o
endif

ifeq ($(CONFIG_chooser_simple),y)
OBJECTS += $(BUILD_DIR)/choosers/simple.o
endif

ifeq ($(CONFIG_chooser_textmenu),y)
OBJECTS += $(BUILD_DIR)/choosers/textmenu.o
endif

OBJECTS +=  \
	$(BUILD_DIR)/$(ARCH)/bzimage.o \
	$(BUILD_DIR)/$(ARCH)/config.o \
	$(BUILD_DIR)/$(ARCH)/gzip.o \
	$(BUILD_DIR)/$(ARCH)/gzip_loader.o \
	$(BUILD_DIR)/$(ARCH)/plain_loader.o \
	$(BUILD_DIR)/$(ARCH)/system.o
	


OBJECTS +=  \
	$(BUILD_DIR)/devschemes/devschemes.o \
	$(BUILD_DIR)/$(FILESYSTEMS)


#
# Source file dependencies
#

$(BUILD_DIR)/elilo.o :  $(INC_DEPS)
$(BUILD_DIR)/elilo.o :
$(BUILD_DIR)/getopt.o :  $(INC_DEPS)
$(BUILD_DIR)/strops.o :  $(INC_DEPS)
$(BUILD_DIR)/loader.o :  $(INC_DEPS)
$(BUILD_DIR)/fileops.o :  $(INC_DEPS)
$(BUILD_DIR)/util.o :  $(INC_DEPS)
$(BUILD_DIR)/vars.o :  $(INC_DEPS)
$(BUILD_DIR)/alloc.o :  $(INC_DEPS)
$(BUILD_DIR)/chooser.o :  $(INC_DEPS)
$(BUILD_DIR)/config.o :  $(INC_DEPS)
$(BUILD_DIR)/initrd.o :  $(INC_DEPS)
$(BUILD_DIR)/alternate.o :  $(INC_DEPS)
$(BUILD_DIR)/bootparams.o :  $(INC_DEPS)
$(BUILD_DIR)/gunzip.o :  $(INC_DEPS)
$(BUILD_DIR)/console.o :  $(INC_DEPS)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
