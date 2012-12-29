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

####################################
#  Common inference rules
#
.SUFFIXES : .s .pym

CC_LINE = $(CC) $(C_FLAGS) $(MODULE_CFLAGS) -c $< -o $@
AC_LINE = $(CC) $(ASM_FLAGS) $(MODULE_CFLAGS) -c $< -o $@

%.o:%.c
	$(CC_LINE)

%.o:%.s
	$(AC_LINE)

####################################
#
#  The following will force library targets to be out of date forcing their
#  makefiles to be invoked.  This will always lead to relinking with the
#  library even if the make did not produce a new binary.
#
ifeq ($(flavor FORCE_LIB_CHECK),undefined)
FORCE_LIB =
else
FORCE_LIB = force
endif



force:

####################################
# Common library dependencies
#
ifeq ($(flavor TARGET_LIB),undefined)
$(SDK_BUILD_DIR)/lib/libc/libc.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libc && $(MAKE) -f libc.mak all

$(SDK_BUILD_DIR)/lib/libcpp/libcpp.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libcpp && $(MAKE) -f libcpp.mak all

$(SDK_BUILD_DIR)/lib/libefi/libefi.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libefi && $(MAKE) -f libefi.mak all

$(SDK_BUILD_DIR)/lib/libefishell/libefishell.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libefishell && $(MAKE) -f libefishell.mak all

$(SDK_BUILD_DIR)/lib/libsocket/libsocket.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libsocket && $(MAKE) -f libsocket.mak all

$(SDK_BUILD_DIR)/lib/libm/libm.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libm && $(MAKE) -f libm.mak all

$(SDK_BUILD_DIR)/lib/libdb/libdb.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libdb && $(MAKE) -f libdb.mak all

$(SDK_BUILD_DIR)/lib/libsmbios/libsmbios.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libsmbios && $(MAKE) -f libsmbios.mak all

$(SDK_BUILD_DIR)/lib/libz/libz.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libz && $(MAKE) -f libz.mak all

$(SDK_BUILD_DIR)/lib/libtty/libtty.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libtty && $(MAKE) -f libtty.mak all

$(SDK_BUILD_DIR)/lib/libmd/libmd.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libmd && $(MAKE) -f libmd.mak all

$(SDK_BUILD_DIR)/lib/libcrypt/libcrypt.a : $(FORCE_LIB)
	cd $(SDK_INSTALL_DIR)/lib/libcrypt && $(MAKE) -f libcrypt.mak all
endif

####################################
# Library targes
#

ifdef TARGET_LIB
TARGET_LIB := $(BUILD_DIR)/$(TARGET_LIB).a
BIN_TARGETS +=  $(TARGET_LIB)
$(TARGET_LIB) : $(OBJECTS)
	$(LIB) $(LIB_FLAGS) -r $@ $(OBJECTS)
endif

####################################
# Application targes
#

ifdef TARGET_APP
TARGET_NAME := $(TARGET_APP)
TARGET_APP = $(BIN_DIR)/$(TARGET_NAME).efi
TARGET_EXE = $(BIN_DIR)/$(TARGET_NAME).exe
TARGET_DLL = $(BIN_DIR)/$(TARGET_NAME).dll
BIN_TARGETS +=  $(TARGET_APP)
$(TARGET_APP) : $(OBJECTS) $(LIBS)
	$(LINK) $(LINK_FLAGS_EXE) $(MODULE_LFLAGS) -e $(IMAGE_ENTRY_POINT) -o $(TARGET_EXE) $(OBJECTS) $(LIBS)
	$(LINK) $(LINK_FLAGS_DLL) $(MODULE_LFLAGS) -e $(IMAGE_ENTRY_POINT) -o $(TARGET_DLL) $(OBJECTS) $(LIBS)
	$(FWIMAGE) app $(TARGET_DLL) $(TARGET_APP)
endif

####################################
# Boot service driver targes
#

ifdef TARGET_BS_DRIVER
TARGET_DRIVER  = $(BIN_DIR)/$(TARGET_BS_DRIVER).efi
TARGET_DRV_EXE = $(BIN_DIR)/$(TARGET_BS_DRIVER).exe
TARGET_DRV_DLL = $(BIN_DIR)/$(TARGET_BS_DRIVER).dll
BIN_TARGETS +=  $(TARGET_DRIVER)
$(TARGET_DRIVER) : $(OBJECTS) $(LIBS)
	$(LINK) $(LINK_FLAGS_EXE) $(MODULE_LFLAGS) -e $(IMAGE_ENTRY_POINT) -o $(TARGET_DRV_EXE) $(OBJECTS) $(LIBS)
	$(LINK) $(LINK_FLAGS_DLL) $(MODULE_LFLAGS) -e $(IMAGE_ENTRY_POINT) -o $(TARGET_DRV_DLL) $(OBJECTS) $(LIBS)
	$(FWIMAGE) bsdrv $(TARGET_DRV_DLL) $(TARGET_DRIVER)
endif

####################################
# Runtime service drivers
#

ifdef TARGET_RT_DRIVER
TARGET_DRIVER  = $(BIN_DIR)/$(TARGET_RT_DRIVER).efi
TARGET_DRV_EXE = $(BIN_DIR)/$(TARGET_RT_DRIVER).exe
TARGET_DRV_DLL = $(BIN_DIR)/$(TARGET_RT_DRIVER).dll
BIN_TARGETS += $(TARGET_DRIVER)
$(TARGET_DRIVER) : $(OBJECTS) $(LIBS)
	$(LINK) $(LINK_FLAGS_EXE) $(MODULE_LFLAGS) -e $(IMAGE_ENTRY_POINT) -o $(TARGET_DRV_EXE) $(OBJECTS) $(LIBS)
	$(LINK) $(LINK_FLAGS_DLL) $(MODULE_LFLAGS) -e $(IMAGE_ENTRY_POINT) -o $(TARGET_DRV_DLL) $(OBJECTS) $(LIBS)
	$(FWIMAGE) rtdrv $(TARGET_DRV_DLL) $(TARGET_DRIVER)
endif

####################################
# Common targes
#
all : $(BIN_TARGETS)

dirs : $(BUILD_DIR) $(BIN_DIR)

	
$(BIN_DIR)                : ; - mkdir -p $(BIN_DIR)
$(BUILD_DIR)              : ; - mkdir -p $(BUILD_DIR)
$(BUILD_DIR)/$(PROCESSOR) : ; - mkdir -p $(BUILD_DIR)/$(PROCESSOR)

clean :
#	- rm -rf $(BUILD_DIR)
#	- rm -rf $(BIN_DIR)/$(BASE_NAME).*
	- @rm -rf $(OBJECTS) $(BIN_TARGETS)

