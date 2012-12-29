VAR1 = abc
VAR2 = cde
DIRA = a


ifeq ($(wildcard $(DIRA)/x.txt), )
$(info no exist x.txt)
endif

ifneq ($(wildcard $(DIRA)/a.txt), )
$(info exist $(DIRA)/a.txt)
else
$(info not exist $(DIRA)/a.txt)
endif



all:
	@echo all
