%.o:%.c
	@echo $@ : $<

%.o:%.s
	@echo sss $@ : $<



ifdef TARGET_LIB
TARGET_LIB := $(TARGET_LIB).a
BIN_TARGETS +=  $(TARGET_LIB)
$(TARGET_LIB) : $(OBJECTS)
	@echo xxxxxxxxxxx $(OBJECTS)
endif


all : $(BIN_TARGETS)

