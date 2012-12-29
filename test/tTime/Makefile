EDK_SOURCE=~/EFI-MIPS

PROCESSER=Mips32

VERSION_FLAGS = -DEFI_SPECIFICATION_VERSION=$(EFI_SPECIFICATION_VERSION) \
								-DTIANO_RELEASE_VERSION=$(TIANO_RELEASE_VERSION)

INC = -I $(EDK_SOURCE)/Foundation/Include/$(PROCESSER)      \
	    -I $(EDK_SOURCE)/Foundation/Efi/Include               \
	    -I $(EDK_SOURCE)/Foundation/Include                   \
	    -I $(EDK_SOURCE)/Foundation/Framework/Include         \
	    -I $(EDK_SOURCE)/Foundation/Library/Dxe/Include       \
            -I $(EDK_SOURCE)/Foundation/Efi                       \
            -I $(EDK_SOURCE)/Sample/Universal/Disk/FileSystem/Ext2/Dxe
		  

a : a.c t.c m.c m2.c
	gcc -Wall $(INC) -o a a.c
	gcc -Wall $(INC) -o t t.c
	gcc -Wall $(INC) -o m m.c
	gcc -Wall $(INC) -o m2 m2.c

clean:
	rm -rf a t m m2 *.txt
