TARGET_LIB=tRules

BUILD_DIR := /home/loongson/EFI-MIPS/test/make-test/$(TARGET_LIB)


all : $(OBJECTS)

OBJECTS = $(BUILD_DIR)/b/b.o $(BUILD_DIR)/c.o


$(BUILD_DIR)/b/b.o: b/b.c
$(BUILD_DIR)/c.o: c.s

include master.mak
