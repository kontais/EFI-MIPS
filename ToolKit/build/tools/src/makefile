ifndef EFI_SOURCE
EFI_SOURCE=$(SDK_INSTALL_DIR)
endif

SRC_DIR = $(EFI_SOURCE)/build/tools/src
BIN_DIR = $(EFI_SOURCE)/build/tools/bin

include $(SRC_DIR)/buildtools.env

SUBDIRS = $(SRC_DIR)/syslib \
          $(SRC_DIR)/genmake \
          $(SRC_DIR)/bin_to_h

all: outputdir subdirs

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	make -C $@

#
# 调用方法
# make xxxDIR-clean
#
.PHONY: $(patsubst %,%-clean,$(sort $(SUBDIRS)))
$(patsubst %,%-clean,$(sort $(SUBDIRS))):
	make -C $(@:-clean=) clean

clean: $(patsubst %,%-clean,$(sort $(SUBDIRS)))

#
# Create BIN_DIR dir
#
.PHONY: outputdir
outputdir:
	@if [ -d $(BIN_DIR) ] ; then \
  $(ECHO) exist $(BIN_DIR) > $(NULL) ; \
  else \
  $(MKDIR) $(BIN_DIR) ; \
  fi

