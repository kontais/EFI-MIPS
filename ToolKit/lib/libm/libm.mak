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

BASE_NAME = libm

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
BUILD_DIR = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
#BUILD_DIR  = $(SDK_BUILD_DIR)/lib/$(BASE_NAME)

#
# Additional compiler flags
#

#C_FLAGS += -D__STDC__

#
# Include paths
#

include $(SOURCE_DIR)/makefile.hdr

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

include $(SDK_INSTALL_DIR)/lib/libm/common_source/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/lib/libm/common_source

#
# Default targets
#

#all : $(OBJECTS)
all : $(OBJECTS)

sub_dirs: $(BUILD_DIR)/efi \
          $(BUILD_DIR)/common \
          $(BUILD_DIR)/common_source \
          $(BUILD_DIR)/ieee \
          $(BUILD_DIR)/gen

#
# Sub-directory targets
#

$(BUILD_DIR)/efi           : ; - rm -rf $(BUILD_DIR)/efi
$(BUILD_DIR)/common        : ; - rm -rf $(BUILD_DIR)/common
$(BUILD_DIR)/common_source : ; - rm -rf $(BUILD_DIR)/common_source
$(BUILD_DIR)/ieee          : ; - rm -rf $(BUILD_DIR)/ieee
$(BUILD_DIR)/gen           : ; - rm -rf $(BUILD_DIR)/gen

#
#  Library object files
#

OBJECTS +=  \
    $(BUILD_DIR)/efi/init.o \
    $(BUILD_DIR)/efi/InitFP.o \
\
    $(BUILD_DIR)/common/atan2.o \
    $(BUILD_DIR)/common/sincos.o \
    $(BUILD_DIR)/common/tan.o \
\
    $(BUILD_DIR)/common_source/acosh.o \
    $(BUILD_DIR)/common_source/asincos.o \
    $(BUILD_DIR)/common_source/asinh.o \
    $(BUILD_DIR)/common_source/atan.o \
    $(BUILD_DIR)/common_source/atanh.o \
    $(BUILD_DIR)/common_source/cosh.o \
    $(BUILD_DIR)/common_source/erf.o \
    $(BUILD_DIR)/common_source/exp.o \
    $(BUILD_DIR)/common_source/exp__E.o \
    $(BUILD_DIR)/common_source/expm1.o \
    $(BUILD_DIR)/common_source/floor.o \
    $(BUILD_DIR)/common_source/fmod.o \
    $(BUILD_DIR)/common_source/gamma.o \
    $(BUILD_DIR)/common_source/j0.o \
    $(BUILD_DIR)/common_source/j1.o \
    $(BUILD_DIR)/common_source/jn.o \
    $(BUILD_DIR)/common_source/lgamma.o \
    $(BUILD_DIR)/common_source/log.o \
    $(BUILD_DIR)/common_source/log__L.o \
    $(BUILD_DIR)/common_source/log10.o \
    $(BUILD_DIR)/common_source/log1p.o \
    $(BUILD_DIR)/common_source/pow.o \
    $(BUILD_DIR)/common_source/sinh.o \
    $(BUILD_DIR)/common_source/tanh.o \
\
    $(BUILD_DIR)/ieee/cabs.o \
    $(BUILD_DIR)/ieee/cbrt.o \
    $(BUILD_DIR)/ieee/support.o \
\
    $(BUILD_DIR)/gen/fabs.o \
    $(BUILD_DIR)/gen/frexp.o \
    $(BUILD_DIR)/gen/ldexp.o \
    $(BUILD_DIR)/gen/modf.o \
    $(BUILD_DIR)/gen/abs.o \
    $(BUILD_DIR)/gen/atof.o

#
# Source file dependencies
#

$(BUILD_DIR)/efi/init.o              : efi/init.c           $(INC_DEPS)
$(BUILD_DIR)/efi/InitFP.o            : efi/InitFP.s         $(INC_DEPS)

$(BUILD_DIR)/common/atan2.o          : common/atan2.c        $(INC_DEPS)
$(BUILD_DIR)/common/sincos.o         : common/sincos.c        $(INC_DEPS)
$(BUILD_DIR)/common/tan.o            : common/tan.c        $(INC_DEPS)

$(BUILD_DIR)/common_source/acosh.o   : common_source/acosh.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/asincos.o : common_source/asincos.c $(INC_DEPS)
$(BUILD_DIR)/common_source/asinh.o   : common_source/asinh.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/atan.o    : common_source/atan.c    $(INC_DEPS)
$(BUILD_DIR)/common_source/atanh.o   : common_source/atanh.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/cosh.o    : common_source/cosh.c    $(INC_DEPS)
$(BUILD_DIR)/common_source/erf.o     : common_source/erf.c     $(INC_DEPS)
$(BUILD_DIR)/common_source/exp.o     : common_source/exp.c     $(INC_DEPS)
$(BUILD_DIR)/common_source/exp__E.o  : common_source/exp__E.c  $(INC_DEPS)
$(BUILD_DIR)/common_source/expm1.o   : common_source/expm1.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/floor.o   : common_source/floor.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/fmod.o    : common_source/fmod.c    $(INC_DEPS)
$(BUILD_DIR)/common_source/gamma.o   : common_source/gamma.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/j0.o      : common_source/j0.c      $(INC_DEPS)
$(BUILD_DIR)/common_source/j1.o      : common_source/j1.c      $(INC_DEPS)
$(BUILD_DIR)/common_source/jn.o      : common_source/jn.c      $(INC_DEPS)
$(BUILD_DIR)/common_source/lgamma.o  : common_source/lgamma.c  $(INC_DEPS)
$(BUILD_DIR)/common_source/log.o     : common_source/log.c     $(INC_DEPS)
$(BUILD_DIR)/common_source/log__L.o  : common_source/log__L.c  $(INC_DEPS)
$(BUILD_DIR)/common_source/log10.o   : common_source/log10.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/log1p.o   : common_source/log1p.c   $(INC_DEPS)
$(BUILD_DIR)/common_source/pow.o     : common_source/pow.c     $(INC_DEPS)
$(BUILD_DIR)/common_source/sinh.o    : common_source/sinh.c    $(INC_DEPS)
$(BUILD_DIR)/common_source/tanh.o    : common_source/tanh.c    $(INC_DEPS)

$(BUILD_DIR)/ieee/cabs.o             : ieee/cabs.c             $(INC_DEPS)
$(BUILD_DIR)/ieee/cbrt.o             : ieee/cbrt.c             $(INC_DEPS)
$(BUILD_DIR)/ieee/support.o          : ieee/support.c          $(INC_DEPS)

$(BUILD_DIR)/gen/fabs.o              : gen/fabs.c            $(INC_DEPS)
$(BUILD_DIR)/gen/frexp.o             : gen/frexp.c           $(INC_DEPS)
$(BUILD_DIR)/gen/ldexp.o             : gen/ldexp.c           $(INC_DEPS)
$(BUILD_DIR)/gen/modf.o              : gen/modf.c            $(INC_DEPS)
$(BUILD_DIR)/gen/abs.o               : gen/abs.c             $(INC_DEPS)
$(BUILD_DIR)/gen/atof.o              : gen/atof.c            $(INC_DEPS)


#
# Handoff to Master.Mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
