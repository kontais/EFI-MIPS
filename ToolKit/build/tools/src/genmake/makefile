ifndef EFI_SOURCE
EFI_SOURCE=$(SDK_INSTALL_DIR)
endif

SRC_DIR = $(EFI_SOURCE)/build/tools/src
BIN_DIR = $(EFI_SOURCE)/build/tools/bin

include $(SRC_DIR)/buildtools.env

INC += 
TARGET_EXE_LIBS     = $(BIN_DIR)/syslib.a

TARGET_NAME         = genmake
TARGET_SOURCE_DIR   = $(SRC_DIR)/$(TARGET_NAME)
TARGET_EXE          = $(BIN_DIR)/$(TARGET_NAME)
TARGET_EXE_SOURCE   = $(TARGET_SOURCE_DIR)/genmake.c
TARGET_EXE_INCLUDE  = 

all: $(TARGET_EXE)

#
# Build EXE
#

$(BIN_DIR)/$(TARGET_NAME).o: $(TARGET_EXE_SOURCE) $(TARGET_EXE_INCLUDE)
	$(CC) $(C_FLAGS) $(INC) $(TARGET_EXE_SOURCE) -o $(BIN_DIR)/$(TARGET_NAME).o

$(TARGET_EXE): $(BIN_DIR)/$(TARGET_NAME).o $(LIBS) $(TARGET_EXE_LIBS)
	$(CC) -o $(TARGET_EXE) $(BIN_DIR)/$(TARGET_NAME).o $(LIBS) $(TARGET_LIB)  $(TARGET_EXE_LIBS) 

clean:
	$(RM) $(BIN_DIR)/$(TARGET_NAME)*

