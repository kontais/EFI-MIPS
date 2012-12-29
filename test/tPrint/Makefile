EDK_SOURCE=~/EFI-MIPS

PROCESSER=Mips32

VERSION_FLAGS = -DEFI_SPECIFICATION_VERSION=$(EFI_SPECIFICATION_VERSION) \
								-DTIANO_RELEASE_VERSION=$(TIANO_RELEASE_VERSION)

INC = -I $(EDK_SOURCE)/Foundation/Include/$(PROCESSER)      \
	    -I $(EDK_SOURCE)/Foundation/Efi/Include               \
	    -I $(EDK_SOURCE)/Foundation/Include                   \
	    -I $(EDK_SOURCE)/Foundation/Cpu/Loongson/Include/Mips32      \
	    -I $(EDK_SOURCE)/Foundation/Cpu/Loongson/Include      \
	    -I $(EDK_SOURCE)/Foundation/Framework/Include         \
	    -I $(EDK_SOURCE)/Foundation/Library/Dxe/Include       \
            -I $(EDK_SOURCE)/Foundation/Efi                       \
            -I $(EDK_SOURCE)/Sample/Universal/Disk/FileSystem/Ext2/Dxe
		  

a : a.c m.s
	gcc -g -mabi=32 -mips3 $(INC) -P -xassembler-with-cpp -c -o m.o m.s
	gcc -g -Wall -fshort-wchar $(INC) -o a a.c m.o
	objdump -D a > a.dump

clean:
	rm -rf a  *.o *.txt a.dump
