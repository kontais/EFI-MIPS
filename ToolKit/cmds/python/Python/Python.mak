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

BASE_NAME = Python

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
#  Local include dependencies
#

INC_DEPS +=  \
	importdl.h 

#
#  Library object files
#

OBJECTS +=  \
    $(BUILD_DIR)/bltinmodule.o \
    $(BUILD_DIR)/ceval.o \
    $(BUILD_DIR)/codecs.o \
    $(BUILD_DIR)/compile.o \
    $(BUILD_DIR)/dynload_stub.o \
    $(BUILD_DIR)/errors.o \
    $(BUILD_DIR)/exceptions.o \
    $(BUILD_DIR)/frozen.o \
    $(BUILD_DIR)/frozenmain.o \
    $(BUILD_DIR)/future.o \
    $(BUILD_DIR)/getargs.o \
    $(BUILD_DIR)/getcompiler.o \
    $(BUILD_DIR)/getcopyright.o \
    $(BUILD_DIR)/getmtime.o \
    $(BUILD_DIR)/getopt.o \
    $(BUILD_DIR)/getplatform.o \
    $(BUILD_DIR)/getversion.o \
    $(BUILD_DIR)/graminit.o \
    $(BUILD_DIR)/import.o \
    $(BUILD_DIR)/importdl.o \
    $(BUILD_DIR)/marshal.o \
    $(BUILD_DIR)/modsupport.o \
    $(BUILD_DIR)/mysnprintf.o \
    $(BUILD_DIR)/mystrtoul.o \
    $(BUILD_DIR)/pyfpe.o \
    $(BUILD_DIR)/pystate.o \
    $(BUILD_DIR)/pystrtod.o \
    $(BUILD_DIR)/pythonrun.o \
    $(BUILD_DIR)/sigcheck.o \
    $(BUILD_DIR)/structmember.o \
    $(BUILD_DIR)/symtable.o \
    $(BUILD_DIR)/sysmodule.o \
    $(BUILD_DIR)/traceback.o \
    $(BUILD_DIR)/EfiInterface.o 

#
# Source file dependencies
#

$(BUILD_DIR)/bltinmodule.o  :  $(INC_DEPS)
$(BUILD_DIR)/ceval.o        :  $(INC_DEPS)
$(BUILD_DIR)/compile.o      :  $(INC_DEPS)
$(BUILD_DIR)/errors.o       :  $(INC_DEPS)
$(BUILD_DIR)/frozen.o       :  $(INC_DEPS)
$(BUILD_DIR)/frozenmain.o   :  $(INC_DEPS)
$(BUILD_DIR)/getargs.o      :  $(INC_DEPS)
$(BUILD_DIR)/getcompiler.o  :  $(INC_DEPS)
$(BUILD_DIR)/getcopyright.o :  $(INC_DEPS)
$(BUILD_DIR)/getmtime.o     :  $(INC_DEPS)
$(BUILD_DIR)/getplatform.o  :  $(INC_DEPS)
$(BUILD_DIR)/getversion.o   :  $(INC_DEPS)
$(BUILD_DIR)/graminit.o     :  $(INC_DEPS)
$(BUILD_DIR)/import.o       :  $(INC_DEPS)
$(BUILD_DIR)/importdl.o     :  $(INC_DEPS)
$(BUILD_DIR)/marshal.o      :  $(INC_DEPS)
$(BUILD_DIR)/modsupport.o   :  $(INC_DEPS)
$(BUILD_DIR)/mystrtoul.o    :  $(INC_DEPS)
$(BUILD_DIR)/pyfpe.o        :  $(INC_DEPS)
$(BUILD_DIR)/pystate.o      :  $(INC_DEPS)
$(BUILD_DIR)/pythonrun.o    :  $(INC_DEPS)
$(BUILD_DIR)/structmember.o :  $(INC_DEPS)
$(BUILD_DIR)/sysmodule.o    :  $(INC_DEPS)
$(BUILD_DIR)/traceback.o    :  $(INC_DEPS)
$(BUILD_DIR)/sigcheck.o     :  $(INC_DEPS)
$(BUILD_DIR)/exceptions.o   :  $(INC_DEPS)
$(BUILD_DIR)/getopt.o		  :  $(INC_DEPS)
$(BUILD_DIR)/codecs.o		  :  $(INC_DEPS)
$(BUILD_DIR)/mysnprintf.o	  :  $(INC_DEPS)
$(BUILD_DIR)/pystrtod.o	  :  $(INC_DEPS)
$(BUILD_DIR)/future.o		  :  $(INC_DEPS)
$(BUILD_DIR)/symtable.o	  :  $(INC_DEPS)
$(BUILD_DIR)/dynload_stub.o :  $(INC_DEPS)
$(BUILD_DIR)/EfiInterface.o :  $(INC_DEPS)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
