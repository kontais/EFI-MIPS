ifndef EFI_SOURCE
EFI_SOURCE=$(SDK_INSTALL_DIR)
endif

SRC_DIR = $(EFI_SOURCE)/build/tools/src
BIN_DIR = $(EFI_SOURCE)/build/tools/bin

include $(SRC_DIR)/buildtools.env

INC += 
TARGET_LIB_LIBS     =

TARGET_NAME         = syslib
TARGET_SRC_DIR      = $(SRC_DIR)/$(TARGET_NAME)

TARGET_LIB          = $(BIN_DIR)/syslib.a

OBJECTS = $(BIN_DIR)/$(TARGET_NAME).o


all: $(TARGET_LIB)

#
# Build EXE
#

$(BIN_DIR)/$(TARGET_NAME).o: $(TARGET_SRC_DIR)/syslib.c $(TARGET_LIB_INCLUDE)
	$(CC) $(C_FLAGS) $(INC) $< -o $@

$(TARGET_LIB): $(BIN_DIR)/$(TARGET_NAME).o $(LIBS) $(TARGET_LIB_LIBS)
	$(LIB) $(LIB_FLAGS) r $(TARGET_LIB) $(OBJECTS)

clean:
	$(RM) $(BIN_DIR)/$(TARGET_NAME)*

