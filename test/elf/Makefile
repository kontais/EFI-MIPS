all : ElfLoader.o elf.txt elf.dump a.dump
ElfLoader.o :
	gcc -Wall -G0 -mno-abicalls -fno-pic -c -o ElfLoader.o -I/home/loongson/EFI-MIPS/Foundation  -I/home/loongson/EFI-MIPS/Foundation/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Framework  -I/home/loongson/EFI-MIPS/Foundation/Framework/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Efi  -I/home/loongson/EFI-MIPS/Foundation/Efi/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Include  -I/home/loongson/EFI-MIPS/Foundation/Include/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Efi/Include  -I/home/loongson/EFI-MIPS/Foundation/Efi/Include/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Framework/Include  -I/home/loongson/EFI-MIPS/Foundation/Framework/Include/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Include/IndustryStandard  -I/home/loongson/EFI-MIPS/Foundation/Include/IndustryStandard/Mips32  -I/home/loongson/EFI-MIPS  -I/home/loongson/EFI-MIPS/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Core/Dxe  -I/home/loongson/EFI-MIPS/Foundation/Core/Dxe/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Library/Dxe/Include  -I/home/loongson/EFI-MIPS/Foundation/Library/Dxe/Include/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Include/Pei  -I/home/loongson/EFI-MIPS/Foundation/Include/Pei/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Library/Pei/Include  -I/home/loongson/EFI-MIPS/Foundation/Library/Pei/Include/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Core/Pei/Include  -I/home/loongson/EFI-MIPS/Foundation/Core/Pei/Include/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Framework/Ppi/CpuIo  -I/home/loongson/EFI-MIPS/Foundation/Framework/Ppi/CpuIo/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Framework  -I/home/loongson/EFI-MIPS/Foundation/Framework/Mips32  -I/home/loongson/EFI-MIPS/Foundation/Library/ElfLoader/Include  -I/home/loongson/EFI-MIPS/Foundation/Library/ElfLoader/Include/Mips32  -DEFI_SPECIFICATION_VERSION=0x00020000 -DTIANO_RELEASE_VERSION=0x00080006 -DEFI_DEBUG -DEFI_DEBUG_CLEAR_MEMORY -DEFI_PEI_PERFORMANCE -DEFI_DXE_PERFORMANCE -DEFI_DXE_PERFORMANCE -DEFI_PEI_REPORT_STATUS_CODE_ON -march=loongson2e -mtune=loongson2e -mabi=32 -mips3 -EL -DEFI32 -DUNICODE -DEFI_LINUX_EMULATOR -DNEW_PEI_HOBS -nostdinc -nostdlib -fno-builtin ElfLoader.c

elf.txt : ElfLoader.o
	readelf -a ElfLoader.o > $@

elf.dump : ElfLoader.o
	objdump -Dtr -s ElfLoader.o > $@

a.dump : ElfLoader.o
	ld ElfLoader.o
	objdump -Dtr -s a.out > a.dump

clean:
	rm -rf ElfLoader.o elf.* a.out a.dump
