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

BASE_NAME = libsocket

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)

#
# Additional compiler flags
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

include $(SOURCE_DIR)/makefile.hdr
INC += -I .


#
# Default targets
#

all : $(OBJECTS)

#
# Library object files
#

OBJECTS +=  \
    $(BUILD_DIR)/EfiSocketInit.o \
    $(BUILD_DIR)/EfiSocketIo.o \
    $(BUILD_DIR)/accept.o \
    $(BUILD_DIR)/addr2ascii.o \
    $(BUILD_DIR)/ascii2addr.o \
    $(BUILD_DIR)/base64.o \
    $(BUILD_DIR)/bind.o \
    $(BUILD_DIR)/connect.o \
    $(BUILD_DIR)/ether_addr.o \
    $(BUILD_DIR)/gethostbydns.o \
    $(BUILD_DIR)/gethostbyht.o \
    $(BUILD_DIR)/gethostbynis.o \
    $(BUILD_DIR)/gethostnamadr.o \
    $(BUILD_DIR)/gethostname.o \
    $(BUILD_DIR)/getnetbydns.o \
    $(BUILD_DIR)/getnetbyht.o \
    $(BUILD_DIR)/getnetbynis.o \
    $(BUILD_DIR)/getnetnamadr.o \
    $(BUILD_DIR)/getpeername.o \
    $(BUILD_DIR)/getproto.o \
    $(BUILD_DIR)/getprotoent.o \
    $(BUILD_DIR)/getprotoname.o \
    $(BUILD_DIR)/getservbyname.o \
    $(BUILD_DIR)/getservbyport.o \
    $(BUILD_DIR)/getservent.o \
    $(BUILD_DIR)/getsockname.o \
    $(BUILD_DIR)/getsockopt.o \
    $(BUILD_DIR)/herror.o \
    $(BUILD_DIR)/inet_addr.o \
    $(BUILD_DIR)/inet_lnaof.o \
    $(BUILD_DIR)/inet_makeaddr.o \
    $(BUILD_DIR)/inet_net_ntop.o \
    $(BUILD_DIR)/inet_net_pton.o \
    $(BUILD_DIR)/inet_neta.o \
    $(BUILD_DIR)/inet_netof.o \
    $(BUILD_DIR)/inet_network.o \
    $(BUILD_DIR)/inet_ntoa.o \
    $(BUILD_DIR)/inet_ntop.o \
    $(BUILD_DIR)/inet_pton.o \
    $(BUILD_DIR)/linkaddr.o \
    $(BUILD_DIR)/listen.o \
    $(BUILD_DIR)/map_v4v6.o \
    $(BUILD_DIR)/ns_addr.o \
    $(BUILD_DIR)/ns_name.o \
    $(BUILD_DIR)/ns_netint.o \
    $(BUILD_DIR)/ns_ntoa.o \
    $(BUILD_DIR)/ns_parse.o \
    $(BUILD_DIR)/ns_print.o \
    $(BUILD_DIR)/ns_ttl.o \
    $(BUILD_DIR)/nsap_addr.o \
    $(BUILD_DIR)/pollsocket.o \
    $(BUILD_DIR)/recv.o \
    $(BUILD_DIR)/recvfrom.o \
    $(BUILD_DIR)/res_comp.o \
    $(BUILD_DIR)/res_data.o \
    $(BUILD_DIR)/res_debug.o \
    $(BUILD_DIR)/res_init.o \
    $(BUILD_DIR)/res_mkquery.o \
    $(BUILD_DIR)/res_mkupdate.o \
    $(BUILD_DIR)/res_query.o \
    $(BUILD_DIR)/res_send.o \
    $(BUILD_DIR)/res_update.o \
    $(BUILD_DIR)/send.o \
    $(BUILD_DIR)/sendto.o \
    $(BUILD_DIR)/sethostname.o \
    $(BUILD_DIR)/setsockopt.o \
    $(BUILD_DIR)/shutdown.o \
    $(BUILD_DIR)/socket.o

#
# Source file dependencies
#

$(BUILD_DIR)/EfiSocketInit.o : EfiSocketInit.c $(INC_DEPS)
$(BUILD_DIR)/EfiSocketIo.o   : EfiSocketIo.c   $(INC_DEPS)
$(BUILD_DIR)/accept.o        : accept.c        $(INC_DEPS)
$(BUILD_DIR)/addr2ascii.o    : addr2ascii.c    $(INC_DEPS)
$(BUILD_DIR)/ascii2addr.o    : ascii2addr.c    $(INC_DEPS)
$(BUILD_DIR)/base64.o        : base64.c        $(INC_DEPS)
$(BUILD_DIR)/bind.o          : bind.c          $(INC_DEPS)
$(BUILD_DIR)/connect.o       : connect.c       $(INC_DEPS)
$(BUILD_DIR)/ether_addr.o    : ether_addr.c    $(INC_DEPS)
$(BUILD_DIR)/gethostbydns.o  : gethostbydns.c  $(INC_DEPS)
$(BUILD_DIR)/gethostbyht.o   : gethostbyht.c   $(INC_DEPS)
$(BUILD_DIR)/gethostbynis.o  : gethostbynis.c  $(INC_DEPS)
$(BUILD_DIR)/gethostnamadr.o : gethostnamadr.c $(INC_DEPS)
$(BUILD_DIR)/gethostname.o   : gethostname.c   $(INC_DEPS)
$(BUILD_DIR)/getnetbydns.o   : getnetbydns.c   $(INC_DEPS)
$(BUILD_DIR)/getnetbyht.o    : getnetbyht.c    $(INC_DEPS)
$(BUILD_DIR)/getnetbynis.o   : getnetbynis.c   $(INC_DEPS)
$(BUILD_DIR)/getnetnamadr.o  : getnetnamadr.c  $(INC_DEPS)
$(BUILD_DIR)/getpeername.o   : getpeername.c   $(INC_DEPS)
$(BUILD_DIR)/getproto.o      : getproto.c      $(INC_DEPS)
$(BUILD_DIR)/getprotoent.o   : getprotoent.c   $(INC_DEPS)
$(BUILD_DIR)/getprotoname.o  : getprotoname.c  $(INC_DEPS)
$(BUILD_DIR)/getservbyname.o : getservbyname.c $(INC_DEPS)
$(BUILD_DIR)/getservbyport.o : getservbyport.c $(INC_DEPS)
$(BUILD_DIR)/getservent.o    : getservent.c    $(INC_DEPS)
$(BUILD_DIR)/getsockname.o   : getsockname.c   $(INC_DEPS)
$(BUILD_DIR)/getsockopt.o    : getsockopt.c    $(INC_DEPS)
$(BUILD_DIR)/herror.o        : herror.c        $(INC_DEPS)
$(BUILD_DIR)/inet_addr.o     : inet_addr.c     $(INC_DEPS)
$(BUILD_DIR)/inet_lnaof.o    : inet_lnaof.c    $(INC_DEPS)
$(BUILD_DIR)/inet_makeaddr.o : inet_makeaddr.c $(INC_DEPS)
$(BUILD_DIR)/inet_net_ntop.o : inet_net_ntop.c $(INC_DEPS)
$(BUILD_DIR)/inet_net_pton.o : inet_net_pton.c $(INC_DEPS)
$(BUILD_DIR)/inet_neta.o     : inet_neta.c     $(INC_DEPS)
$(BUILD_DIR)/inet_netof.o    : inet_netof.c    $(INC_DEPS)
$(BUILD_DIR)/inet_network.o  : inet_network.c  $(INC_DEPS)
$(BUILD_DIR)/inet_ntoa.o     : inet_ntoa.c     $(INC_DEPS)
$(BUILD_DIR)/inet_ntop.o     : inet_ntop.c     $(INC_DEPS)
$(BUILD_DIR)/inet_pton.o     : inet_pton.c     $(INC_DEPS)
$(BUILD_DIR)/linkaddr.o      : linkaddr.c      $(INC_DEPS)
$(BUILD_DIR)/listen.o        : listen.c        $(INC_DEPS)
$(BUILD_DIR)/map_v4v6.o      : map_v4v6.c      $(INC_DEPS)
$(BUILD_DIR)/ns_addr.o       : ns_addr.c       $(INC_DEPS)
$(BUILD_DIR)/ns_name.o       : ns_name.c       $(INC_DEPS)
$(BUILD_DIR)/ns_netint.o     : ns_netint.c     $(INC_DEPS)
$(BUILD_DIR)/ns_ntoa.o       : ns_ntoa.c       $(INC_DEPS)
$(BUILD_DIR)/ns_parse.o      : ns_parse.c      $(INC_DEPS)
$(BUILD_DIR)/ns_print.o      : ns_print.c      $(INC_DEPS)
$(BUILD_DIR)/ns_ttl.o        : ns_ttl.c        $(INC_DEPS)
$(BUILD_DIR)/nsap_addr.o     : nsap_addr.c     $(INC_DEPS)
$(BUILD_DIR)/pollsocket.o    : pollsocket.c    $(INC_DEPS)
$(BUILD_DIR)/recv.o          : recv.c          $(INC_DEPS)
$(BUILD_DIR)/recvfrom.o      : recvfrom.c      $(INC_DEPS)
$(BUILD_DIR)/res_comp.o      : res_comp.c      $(INC_DEPS)
$(BUILD_DIR)/res_data.o      : res_data.c      $(INC_DEPS)
$(BUILD_DIR)/res_debug.o     : res_debug.c     $(INC_DEPS)
$(BUILD_DIR)/res_init.o      : res_init.c      $(INC_DEPS)
$(BUILD_DIR)/res_mkquery.o   : res_mkquery.c   $(INC_DEPS)
$(BUILD_DIR)/res_mkupdate.o  : res_mkupdate.c  $(INC_DEPS)
$(BUILD_DIR)/res_query.o     : res_query.c     $(INC_DEPS)
$(BUILD_DIR)/res_send.o      : res_send.c      $(INC_DEPS)
$(BUILD_DIR)/res_update.o    : res_update.c    $(INC_DEPS)
$(BUILD_DIR)/send.o          : send.c          $(INC_DEPS)
$(BUILD_DIR)/sendto.o        : sendto.c        $(INC_DEPS)
$(BUILD_DIR)/sethostname.o   : sethostname.c   $(INC_DEPS)
$(BUILD_DIR)/setsockopt.o    : setsockopt.c    $(INC_DEPS)
$(BUILD_DIR)/shutdown.o      : shutdown.c      $(INC_DEPS)
$(BUILD_DIR)/socket.o        : socket.c        $(INC_DEPS)

#
# Handoff to Master.Mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
