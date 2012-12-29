#/*++
#
#  Copyright (c) 2009, kontais                                                                          
#  Copyright (c) 2004 - 2007, Intel Corporation                                                         
#  All rights reserved. This program and the accompanying materials                          
#  are licensed and made available under the terms and conditions of the BSD License         
#  which accompanies this distribution.  The full text of the license may be found at        
#  http://opensource.org/licenses/bsd-license.php                                            
#                                                                                            
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
#  
#  Module Name:  makefile
#
#  Abstract:
#
#    This file is used to build the EFI build tools.
#
#--*/

#
# Everything depends on EDK_SOURCE. Make sure it's defined
#
ifndef EDK_SOURCE
$(error EDK_SOURCE environmental variable not set)
endif

#
# Define our toolchain before we include the master settings file
#
TOOLCHAIN = TOOLCHAIN_GCC

include $(BUILD_DIR)/PlatformTools.env

#
# Define all the makefiles we want to call
#

SUBDIRS =   $(EDK_TOOLS_SOURCE)/Common                      \
            $(CUSTOMIZEDCOMPRESS_MAKEFILE)                  \
            $(EDK_TOOLS_SOURCE)/GenCRC32Section             \
            $(EDK_TOOLS_SOURCE)/GenSection                  \
            $(EDK_TOOLS_SOURCE)/GenDepex                    \
            $(EDK_TOOLS_SOURCE)/GenFfsFile                  \
            $(EDK_TOOLS_SOURCE)/GenFvImage                  \
            $(EDK_TOOLS_SOURCE)/ElfLink                     \
            $(EDK_TOOLS_SOURCE)/FwImage                     \
            $(EDK_TOOLS_SOURCE)/ProcessDsc                  \
            $(EDK_TOOLS_SOURCE)/GuidChk                     \
            $(EDK_TOOLS_SOURCE)/MakeDeps                    \
            $(EDK_TOOLS_SOURCE)/SetStamp                    \
            $(EDK_TOOLS_SOURCE)/VfrCompile                  \
            $(EDK_TOOLS_SOURCE)/StrGather                   \
            $(EDK_TOOLS_SOURCE)/BootsectImage               \
            $(EDK_TOOLS_SOURCE)/GenPage                     \
            $(EDK_TOOLS_SOURCE)/SplitFile                   \
            $(EDK_TOOLS_SOURCE)/ElfToBin                    \
            $(EDK_TOOLS_SOURCE)/EfiCompress                 \
            $(EDK_TOOLS_SOURCE)/EfildrImage                 \
            $(EDK_TOOLS_SOURCE)/EfiRom                      \
            $(EDK_TOOLS_SOURCE)/GenAprioriFile         

#            $(EDK_TOOLS_SOURCE)/GenBootsector               \ 没有移植

#
# Define default all target which calls all our makefiles. The special
# bang (!) tells nmake to do the command for each out-of-date dependent.
#
# Create the BIN directory, which will only exist if you pull the source tree
# from version control.
#
all: outputdir subdirs

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	make -I $(BUILD_DIR) -C $@

#
# 调用方法
# make xxxDIR-clean
#
.PHONY: $(patsubst %,%-clean,$(sort $(SUBDIRS)))
$(patsubst %,%-clean,$(sort $(SUBDIRS))):
	make -C $(@:-clean=) clean

clean: $(patsubst %,%-clean,$(sort $(SUBDIRS)))

#
# Create EDK_TOOLS_OUTPUT dir
#
.PHONY: outputdir
outputdir:
	@if [ -d $(EDK_TOOLS_OUTPUT) ] ; then \
	$(ECHO) exist $(EDK_TOOLS_OUTPUT) > $(NULL) ; \
	else \
	$(MKDIR) $(EDK_TOOLS_OUTPUT) ; \
	fi
