#
# Copyright (c) 2000
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

BASE_NAME = dhclient

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
# Additional compile flags
#

#C_FLAGS +=  -D __FreeBSD__ -D __STDC__
C_FLAGS +=  -D__FreeBSD__

#
# Uncomment the next line if the DHCP client should support
# maintaining lease files.  This allows the client to request
# the same IP address it used the last time it ran.
#
C_FLAGS +=  -DLEASE_FILE_SUPPORT

#
# This next line is for debug.  It will print to the screen when
# leases are renewed or changed.
#
#C_FLAGS +=  -D BACKGROUND_DISPLAY

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

INC += -I . \
      -I ./includes

#
# Libraries
#

LIBS +=  \
       $(SDK_INSTALL_DIR)/lib/libsocket/libsocket.a \
       $(SDK_INSTALL_DIR)/lib/libc/libc.a
       
all : dirs $(LIBS) $(OBJECTS)

#
# Local include dependencies
#

INC_DEPS +=  \
    includes/cdefs.h \
    includes/dhcp.h \
    includes/dhctoken.h \
    includes/hash.h \
    includes/inet.h \
    includes/osdep.h \
    includes/site.h \
    includes/sysconf.h \
    includes/tree.h \
    includes/cf/freebsd.h 

#
# Program object files
#

OBJECTS +=  \
    $(BUILD_DIR)/EfiHook.o \
    $(BUILD_DIR)/client/dhclient.o \
    $(BUILD_DIR)/client/clparse.o \
	$(BUILD_DIR)/common/alloc.o \
	$(BUILD_DIR)/common/conflex.o \
	$(BUILD_DIR)/common/convert.o \
	$(BUILD_DIR)/common/dispatch.o \
	$(BUILD_DIR)/common/errwarn.o \
	$(BUILD_DIR)/common/hash.o \
	$(BUILD_DIR)/common/inet.o \
	$(BUILD_DIR)/common/memory.o \
	$(BUILD_DIR)/common/options.o \
	$(BUILD_DIR)/common/packet.o \
	$(BUILD_DIR)/common/parse.o \
	$(BUILD_DIR)/common/print.o \
	$(BUILD_DIR)/common/socket.o \
	$(BUILD_DIR)/common/tables.o \
	$(BUILD_DIR)/common/tree.o 

#
# Source file dependencies
#

$(BUILD_DIR)/EfiHook.o  :         $(INC_DEPS)
$(BUILD_DIR)/client/dhclient.o :  $(INC_DEPS)
$(BUILD_DIR)/client/clparse.o  :  $(INC_DEPS)
$(BUILD_DIR)/common/alloc.o    :  $(INC_DEPS)
$(BUILD_DIR)/common/conflex.o  :  $(INC_DEPS)
$(BUILD_DIR)/common/convert.o  :  $(INC_DEPS)
$(BUILD_DIR)/common/dispatch.o :  $(INC_DEPS)
$(BUILD_DIR)/common/errwarn.o  :  $(INC_DEPS)
$(BUILD_DIR)/common/hash.o     :  $(INC_DEPS)
$(BUILD_DIR)/common/inet.o     :  $(INC_DEPS)
$(BUILD_DIR)/common/memory.o   :  $(INC_DEPS)
$(BUILD_DIR)/common/options.o  :  $(INC_DEPS)
$(BUILD_DIR)/common/packet.o   :  $(INC_DEPS)
$(BUILD_DIR)/common/parse.o    :  $(INC_DEPS)
$(BUILD_DIR)/common/print.o    :  $(INC_DEPS)
$(BUILD_DIR)/common/socket.o   :  $(INC_DEPS)
$(BUILD_DIR)/common/tables.o   :  $(INC_DEPS)
$(BUILD_DIR)/common/tree.o     :  $(INC_DEPS)



#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
