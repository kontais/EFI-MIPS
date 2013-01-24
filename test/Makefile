MAKEFLAGS += -rR --no-print-directory

SUBDIRS = BinData \
    EFI \
    pack \
    EFI001 \
    elf \
    gp \
    make-test \
    MemCpy \
    MemCpy2 \
    MemCpy3 \
    OK1 \
    OK2 \
    rel \
    sizeof \
    tABI \
    tadjust \
    tAR \
    tassert \
    tbss \
    tCHAR16 \
    tCodeSample \
    tcxx \
    tDebugAssembler \
    tdir \
    tDiv32 \
    tDiv64 \
    tELF \
    tExecRel \
    tfork \
    tframe \
    tFrame2 \
    tFrame3 \
    tgcc \
    tgccConfig \
    tgetenv \
    tguid \
    thash \
    tint64_t \
    tlib \
    tlonglong \
    tMake \
    tMIPS32 \
    tmmap \
    tModule \
    tMulU64 \
    tN32 \
    tobjcopy \
    torg \
    tpic \
    tPredefinedMacros \
    tPrint \
    tRegInit \
    tRelocate \
    tRelType \
    tResetVector \
    tROL-Order \
    tS \
    tSample \
    tS-Byte \
    tshared \
    tsizeof \
    tSrl \
    tstdio \
    tStruct \
    tSwitchStack \
    tSwitchStack2 \
    tswprintf \
    tsys \
    tSysLib \
    tTemplate \
    tTime \
    tU32ToU16 \
    tU64 \
    tuuid \
    twchar_t \
    tweak \
    ulw \
    tlhu \
    tlbu

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

#
# make d1-clean
# make clean
#
.PHONY: $(patsubst %,%-clean,$(sort $(SUBDIRS)))
$(patsubst %,%-clean,$(sort $(SUBDIRS))):
	make -C $(@:-clean=) clean

clean: $(patsubst %,%-clean,$(sort $(SUBDIRS)))

