MAKEFLAGS += -rR --no-print-directory

SUBDIRS = d1 d2

all: subdirs

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	make -C $@

#
# 调用方法
# make d1-clean
#
.PHONY: $(patsubst %,%-clean,$(sort $(SUBDIRS)))
$(patsubst %,%-clean,$(sort $(SUBDIRS))):
	make -C $(@:-clean=) clean

clean: $(patsubst %,%-clean,$(sort $(SUBDIRS)))

