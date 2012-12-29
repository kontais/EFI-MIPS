MAKEFLAGS += -rR --no-print-directory

SUBDIRS = m1.mak m2.mak

all: subdirs

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	make -f $@

clean: cleansubdirs

.PHONY: cleansubdirs $(SUBDIRS)
cleansubdirs: $(SUBDIRS)
$(SUBDIRS):
	make -f $@ clean
