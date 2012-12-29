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

BASE_NAME = libdb

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)

#
# Additional compiler flags
#

#C_FLAGS += -D__STDC__ -D__DBINTERFACE_PRIVATE -DMMAP_NOT_AVAILABLE
C_FLAGS += -D__DBINTERFACE_PRIVATE -DMMAP_NOT_AVAILABLE

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd 

include makefile.hdr
INC += -I btree \
      -I hash \
      -I recno

#
# Default target
#

all : $(OBJECTS)

sub_dirs : $(BUILD_DIR)/btree \
           $(BUILD_DIR)/db \
           $(BUILD_DIR)/hash \
           $(BUILD_DIR)/mpool \
           $(BUILD_DIR)/recno
	
#
# Sub-directory targets
#

$(BUILD_DIR)/btree : ; - md $(BUILD_DIR)/btree
$(BUILD_DIR)/hash  : ; - md $(BUILD_DIR)/hash
$(BUILD_DIR)/mpool : ; - md $(BUILD_DIR)/mpool
$(BUILD_DIR)/recno : ; - md $(BUILD_DIR)/recno
$(BUILD_DIR)/db    : ; - md $(BUILD_DIR)/db

#
#  Library object files
#

OBJECTS +=  \
    $(BUILD_DIR)/btree/bt_close.o \
    $(BUILD_DIR)/btree/bt_conv.o \
    $(BUILD_DIR)/btree/bt_debug.o \
    $(BUILD_DIR)/btree/bt_delete.o \
    $(BUILD_DIR)/btree/bt_get.o \
    $(BUILD_DIR)/btree/bt_open.o \
    $(BUILD_DIR)/btree/bt_overflow.o \
    $(BUILD_DIR)/btree/bt_page.o \
    $(BUILD_DIR)/btree/bt_put.o \
    $(BUILD_DIR)/btree/bt_search.o \
    $(BUILD_DIR)/btree/bt_seq.o \
    $(BUILD_DIR)/btree/bt_split.o \
    $(BUILD_DIR)/btree/bt_utils.o \
\
    $(BUILD_DIR)/db/db.o \
\
    $(BUILD_DIR)/hash/hash.o \
    $(BUILD_DIR)/hash/hash_bigkey.o \
    $(BUILD_DIR)/hash/hash_buf.o \
    $(BUILD_DIR)/hash/hash_func.o \
    $(BUILD_DIR)/hash/hash_log2.o \
    $(BUILD_DIR)/hash/hash_page.o \
    $(BUILD_DIR)/hash/hsearch.o \
    $(BUILD_DIR)/hash/ndbm.o \
\
    $(BUILD_DIR)/mpool/mpool.o \
\
    $(BUILD_DIR)/recno/rec_close.o \
    $(BUILD_DIR)/recno/rec_delete.o \
    $(BUILD_DIR)/recno/rec_get.o \
    $(BUILD_DIR)/recno/rec_open.o \
    $(BUILD_DIR)/recno/rec_put.o \
    $(BUILD_DIR)/recno/rec_search.o \
    $(BUILD_DIR)/recno/rec_seq.o \
    $(BUILD_DIR)/recno/rec_utils.o

#
# Source file dependencies
#

$(BUILD_DIR)/btree/bt_close.o    : btree/bt_close.c    $(INC_DEPS)
$(BUILD_DIR)/btree/bt_conv.o     : btree/bt_conv.c     $(INC_DEPS)
$(BUILD_DIR)/btree/bt_debug.o    : btree/bt_debug.c    $(INC_DEPS)
$(BUILD_DIR)/btree/bt_debug.o    : btree/bt_debug.c    $(INC_DEPS)
$(BUILD_DIR)/btree/bt_delete.o   : btree/bt_delete.c   $(INC_DEPS)
$(BUILD_DIR)/btree/bt_get.o      : btree/bt_get.c      $(INC_DEPS)
$(BUILD_DIR)/btree/bt_open.o     : btree/bt_open.c     $(INC_DEPS)
$(BUILD_DIR)/btree/bt_overflow.o : btree/bt_overflow.c $(INC_DEPS)
$(BUILD_DIR)/btree/bt_page.o     : btree/bt_page.c     $(INC_DEPS)
$(BUILD_DIR)/btree/bt_put.o      : btree/bt_put.c      $(INC_DEPS)
$(BUILD_DIR)/btree/bt_search.o   : btree/bt_search.c   $(INC_DEPS)
$(BUILD_DIR)/btree/bt_seq.o      : btree/bt_seq.c      $(INC_DEPS)
$(BUILD_DIR)/btree/bt_split.o    : btree/bt_split.c    $(INC_DEPS)
$(BUILD_DIR)/btree/bt_utils.o    : btree/bt_utils.c    $(INC_DEPS)

$(BUILD_DIR)/db/db.o             : db/db.c    $(INC_DEPS)

$(BUILD_DIR)/hash/hash.o         : hash/hash.c         $(INC_DEPS)
$(BUILD_DIR)/hash/hash_bigkey.o  : hash/hash_bigkey.c  $(INC_DEPS)
$(BUILD_DIR)/hash/hash_buf.o     : hash/hash_buf.c     $(INC_DEPS)
$(BUILD_DIR)/hash/hash_func.o    : hash/hash_func.c    $(INC_DEPS)
$(BUILD_DIR)/hash/hash_log2.o    : hash/hash_log2.c    $(INC_DEPS)
$(BUILD_DIR)/hash/hash_page.o    : hash/hash_page.c    $(INC_DEPS)
$(BUILD_DIR)/hash/hsearch.o      : hash/hsearch.c      $(INC_DEPS)
$(BUILD_DIR)/hash/ndbm.o         : hash/ndbm.c         $(INC_DEPS)

$(BUILD_DIR)/mpool/mpool.o       : mpool/mpool.c $(INC_DEPS)

$(BUILD_DIR)/recno/rec_close.o   : recno/rec_close.c  $(INC_DEPS)
$(BUILD_DIR)/recno/rec_delete.o  : recno/rec_delete.c $(INC_DEPS)
$(BUILD_DIR)/recno/rec_get.o     : recno/rec_get.c    $(INC_DEPS)
$(BUILD_DIR)/recno/rec_open.o    : recno/rec_open.c   $(INC_DEPS)
$(BUILD_DIR)/recno/rec_put.o     : recno/rec_put.c    $(INC_DEPS)
$(BUILD_DIR)/recno/rec_search.o  : recno/rec_search.c $(INC_DEPS)
$(BUILD_DIR)/recno/rec_seq.o     : recno/rec_seq.c    $(INC_DEPS)
$(BUILD_DIR)/recno/rec_utils.o   : recno/rec_utils.c  $(INC_DEPS)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak

