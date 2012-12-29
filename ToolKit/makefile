#
# Copyright (c) 1999 - 2005
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
ifndef SDK_BUILD_ENV
$(error Please set SDK_BUILD_ENV to "linux32" or  "Mips32")
endif

ifndef SDK_INSTALL_DIR
$(error Please set SDK_INSTALL_DIR to EFI_Toolkit directory) 
endif

SOURCE_DIR=$(SDK_INSTALL_DIR)

include $(SDK_INSTALL_DIR)/build/$(SDK_BUILD_ENV)/sdk.env

#all :
all : build_tools
	cd $(SOURCE_DIR)/lib && $(MAKE) -f lib.mak all
	cd $(SOURCE_DIR)/apps && $(MAKE) -f apps.mak all
	cd $(SOURCE_DIR)/cmds &&$(MAKE) -f cmds.mak all
	cd $(SOURCE_DIR)/protocols && $(MAKE) -f protocols.mak all

clean :
	rm -rf $(SOURCE_DIR)/build/$(SDK_BUILD_ENV)/output
	rm -rf $(SOURCE_DIR)/build/$(SDK_BUILD_ENV)/bin
	cd $(SOURCE_DIR)/lib && $(MAKE) -f lib.mak clean
	cd $(SOURCE_DIR)/apps && $(MAKE) -f apps.mak clean
	cd $(SOURCE_DIR)/cmds && $(MAKE) -f cmds.mak clean
	cd $(SOURCE_DIR)/protocols && $(MAKE) -f protocols.mak clean
	cd $(SOURCE_DIR)/build/tools/src && $(MAKE) -f makefile clean

build_tools:
	cd $(SOURCE_DIR)/build/tools/src && $(MAKE) -f makefile
