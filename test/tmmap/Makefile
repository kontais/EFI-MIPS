CC = gcc

EFI_SPECIFICATION_VERSION = 0x00020000
TIANO_RELEASE_VERSION     = 0x00080006

EDK_SOURCE=/home/loongson/EFI-MIPS

PROCESSER=Mips32

VERSION_FLAGS = -DEFI_SPECIFICATION_VERSION=$(EFI_SPECIFICATION_VERSION) \
								-DTIANO_RELEASE_VERSION=$(TIANO_RELEASE_VERSION)

INC = -I $(EDK_SOURCE)/Foundation/Include/$(PROCESSER)             \
	    -I $(EDK_SOURCE)/Foundation/Efi/Include                      \
	    -I $(EDK_SOURCE)/Foundation/Include                          \
	    -I $(EDK_SOURCE)/Foundation/Framework/Include                \
	    -I $(EDK_SOURCE)/Foundation/Library/Dxe/Include              \
	    -I $(EDK_SOURCE)/Foundation/Cpu/Loongson/Include             \
	    -I $(EDK_SOURCE)/Foundation/Cpu/Loongson/Include/Mips32      \
		  -I $(EDK_SOURCE)/Foundation/Efi

C_PIC_FLAGS       = -G0 -mno-abicalls -fno-pic
#C_PIC_FLAGS       = -G0 -mno-abicalls
#C_PIC_FLAGS       =
ASM_PIC_FLAGS     = $(C_PIC_FLAGS)

C_STD_FLAGS += -Wall $(VERSION_FLAGS) -c -o $@



C_FLAGS += $(C_STD_FLAGS) $(INC)

OBJECTS = Math.o main.o Print.o

all: $(OBJECTS)
	$(CC) -o a $(OBJECTS)

delMath.o : Math.c
	$(CC) -G0 -mno-abicalls  -Wall $(VERSION_FLAGS) $(INC) $< -S -o M.s
	$(CC) -G0 -mno-abicalls $(C_FLAGS) $<

Math.o : Math.s
	$(CC) -P -x assembler-with-cpp $(C_PIC_FLAGS) $(C_FLAGS) $<

Print.o : Print.c
	$(CC) $(C_FLAGS) $<

main.o : main.c
	$(CC) $(C_FLAGS) $<

clean:
	rm -rf $(OBJECTS) a M.s
