TOOLCHAIN =

ifndef TOOLCHAIN
#未定义或示赋值
$(info ifndef no defined)
else
#定义且赋值
$(info ifndef defined)
endif

ifeq ($(flavor TOOLCHAIN),undefined)
$(info flavor no defined)
else
$(info flavor defined)
endif

ifeq ($(origin TOOLCHAIN),undefined)
$(info origin no defined)
else
$(info origin defined)
endif

all:
	@echo done > /dev/null

