var1 = 
var2 = b


all:
ifdef var1
  ifdef var2
	  $(info all defined)
		@echo $(var1) $(var2) > GenStamp.txt
  else
	  @rm -f GenStamp.txt
  endif
else
	@rm -f GenStamp.txt
endif




ifdef var1
$(info var1 defined)
else
$(info var1 no defined )
endif

ifeq ($(origin var1), undefined)
$(info var1 origin undefined)
endif

ifeq ($(flavor var1), undefined)
$(info var1 flavor undefined)
endif


#
# ifdef/ifndef  变量未定义或值为空
#
# $(origin xxx) 只有当变更未定义才是undefined
#

