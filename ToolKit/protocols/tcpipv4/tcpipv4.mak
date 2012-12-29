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

BASE_NAME = tcpipv4

#
# Set entry point
#

IMAGE_ENTRY_POINT = EfiNetEntry

#
# Globals needed by master.mak
#

TARGET_BS_DRIVER = $(BASE_NAME)
SOURCE_DIR       = $(SDK_INSTALL_DIR)/protocols/$(BASE_NAME)
BUILD_DIR        = $(SDK_INSTALL_DIR)/protocols/$(BASE_NAME)

PPP_SUPPORT = YES

#
# Additional compile flags
#

C_FLAGS += -DKERNEL -D__inline__=__inline

ifeq ($(PPP_SUPPORT),YES)

C_FLAGS += -DPPP_SUPPORT

endif

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

LIBS +=  $(SDK_INSTALL_DIR)/lib/libc/libc.a

#
# Main targets
#

all : dirs $(LIBS) $(OBJECTS)

#
# Program object files
#

OBJECTS +=  \
    $(BUILD_DIR)/efi_init.o \
    $(BUILD_DIR)/efi_interface.o \
    $(BUILD_DIR)/efi_kern_support.o \
    $(BUILD_DIR)/efi_netiface.o \
    $(BUILD_DIR)/if.o \
    $(BUILD_DIR)/if_ether.o \
    $(BUILD_DIR)/if_ethersubr.o \
    $(BUILD_DIR)/if_loop.o \
    $(BUILD_DIR)/igmp.o \
    $(BUILD_DIR)/in.o \
    $(BUILD_DIR)/in_cksum.o \
    $(BUILD_DIR)/in_pcb.o \
    $(BUILD_DIR)/in_proto.o \
    $(BUILD_DIR)/in_rmx.o \
    $(BUILD_DIR)/ip_flow.o \
    $(BUILD_DIR)/ip_icmp.o \
    $(BUILD_DIR)/ip_input.o \
    $(BUILD_DIR)/ip_mroute.o \
    $(BUILD_DIR)/ip_output.o \
    $(BUILD_DIR)/kern_timeout.o \
    $(BUILD_DIR)/param.o \
    $(BUILD_DIR)/radix.o \
    $(BUILD_DIR)/random.o \
    $(BUILD_DIR)/raw_cb.o \
    $(BUILD_DIR)/raw_ip.o \
    $(BUILD_DIR)/raw_usrreq.o \
    $(BUILD_DIR)/route.o \
    $(BUILD_DIR)/rtsock.o \
    $(BUILD_DIR)/tcp_debug.o \
    $(BUILD_DIR)/tcp_input.o \
    $(BUILD_DIR)/tcp_output.o \
    $(BUILD_DIR)/tcp_subr.o \
    $(BUILD_DIR)/tcp_timer.o \
    $(BUILD_DIR)/tcp_usrreq.o \
    $(BUILD_DIR)/to_resolve.o \
    $(BUILD_DIR)/udp_usrreq.o \
    $(BUILD_DIR)/uipc_domain.o \
    $(BUILD_DIR)/uipc_mbuf.o \
    $(BUILD_DIR)/uipc_socket.o \
    $(BUILD_DIR)/uipc_socket2.o \
    $(BUILD_DIR)/vm_zone.o 

ifeq ($(PPP_SUPPORT),YES)
OBJECTS +=  \
    $(BUILD_DIR)/if_ppp.o \
    $(BUILD_DIR)/slcompress.o \
    $(BUILD_DIR)/ppp_tty.o \
    $(BUILD_DIR)/tty_subr.o
endif


#
# Source file dependencies
#

$(BUILD_DIR)/efi_init.o         :  $(INC_DEPS)
$(BUILD_DIR)/efi_interface.o    :  $(INC_DEPS)
$(BUILD_DIR)/efi_kern_support.o :  $(INC_DEPS)
$(BUILD_DIR)/efi_netiface.o     :  $(INC_DEPS)
$(BUILD_DIR)/if.o               :  $(INC_DEPS)
$(BUILD_DIR)/if_ether.o         :  $(INC_DEPS)
$(BUILD_DIR)/if_ethersubr.o     :  $(INC_DEPS)
$(BUILD_DIR)/if_loop.o          :  $(INC_DEPS)
$(BUILD_DIR)/igmp.o             :  $(INC_DEPS)
$(BUILD_DIR)/in.o               :  $(INC_DEPS)
$(BUILD_DIR)/in_cksum.o         :  $(INC_DEPS)
$(BUILD_DIR)/in_pcb.o           :  $(INC_DEPS)
$(BUILD_DIR)/in_proto.o         :  $(INC_DEPS)
$(BUILD_DIR)/in_rmx.o           :  $(INC_DEPS)
$(BUILD_DIR)/ip_flow.o          :  $(INC_DEPS)
$(BUILD_DIR)/ip_icmp.o          :  $(INC_DEPS)
$(BUILD_DIR)/ip_input.o         :  $(INC_DEPS)
$(BUILD_DIR)/ip_mroute.o        :  $(INC_DEPS)
$(BUILD_DIR)/ip_output.o        :  $(INC_DEPS)
$(BUILD_DIR)/kern_timeout.o     :  $(INC_DEPS)
$(BUILD_DIR)/param.o            :  $(INC_DEPS)
$(BUILD_DIR)/radix.o            :  $(INC_DEPS)
$(BUILD_DIR)/random.o           :  $(INC_DEPS)
$(BUILD_DIR)/raw_cb.o           :  $(INC_DEPS)
$(BUILD_DIR)/raw_ip.o           :  $(INC_DEPS)
$(BUILD_DIR)/raw_usrreq.o       :  $(INC_DEPS)
$(BUILD_DIR)/route.o            :  $(INC_DEPS)
$(BUILD_DIR)/rtsock.o           :  $(INC_DEPS)
$(BUILD_DIR)/tcp_debug.o        :  $(INC_DEPS)
$(BUILD_DIR)/tcp_input.o        :  $(INC_DEPS)
$(BUILD_DIR)/tcp_output.o       :  $(INC_DEPS)
$(BUILD_DIR)/tcp_subr.o         :  $(INC_DEPS)
$(BUILD_DIR)/tcp_timer.o        :  $(INC_DEPS)
$(BUILD_DIR)/tcp_usrreq.o       :  $(INC_DEPS)
$(BUILD_DIR)/to_resolve.o       :  $(INC_DEPS)
$(BUILD_DIR)/udp_usrreq.o       :  $(INC_DEPS)
$(BUILD_DIR)/uipc_domain.o      :  $(INC_DEPS)
$(BUILD_DIR)/uipc_mbuf.o        :  $(INC_DEPS)
$(BUILD_DIR)/uipc_socket.o      :  $(INC_DEPS)
$(BUILD_DIR)/uipc_socket2.o     :  $(INC_DEPS)
$(BUILD_DIR)/vm_zone.o          :  $(INC_DEPS)
$(BUILD_DIR)/if_ppp.o           :  $(INC_DEPS)
$(BUILD_DIR)/slcompress.o       :  $(INC_DEPS)
$(BUILD_DIR)/ppp_tty.o          :  $(INC_DEPS)
$(BUILD_DIR)/tty_subr.o         :  $(INC_DEPS)

#
# Handoff to Master.Mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
