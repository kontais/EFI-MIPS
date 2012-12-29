sw.o: sw.s
	gcc -P -xassembler-with-cpp -D__ASSEMBLER__  -Wall -fshort-wchar -c -o sw.o -I /home/loongson/EFI-MIPS/Foundation/Cpu/Loongson/Include  -I /home/loongson/EFI-MIPS/Foundation/Cpu/Loongson/Include/Mips32   -DEFI_SPECIFICATION_VERSION=0x00020000 -DTIANO_RELEASE_VERSION=0x00080006 -DEFI_DEBUG -DEFI_DEBUG_CLEAR_MEMORY -DEFI_PEI_PERFORMANCE -DEFI_DXE_PERFORMANCE -DEFI_DXE_PERFORMANCE -DEFI_PEI_REPORT_STATUS_CODE_ON -mabi=32 -mips3 -EL -Wa,-mno-pdr -DEFI32 -G0 -mno-abicalls -fno-pic -G0 -mno-abicalls -fno-pic  -D EFI_LINUX_EMULATOR -nostdinc -nostdlib -fno-builtin sw.s
	objdump -Dtr sw.o > sw.dump
	ld -e main -Ttext=420070 -o s sw.o
	objdump -D s > s.dump
#	gcc -Wall -fshort-wchar -c -o sw.o -mabi=32 -mips3 -EL -Wa,-mno-pdr -G0 -mno-abicalls -fno-pic -nostdinc -nostdlib -fno-builtin sw.c

clean:
	rm -rf sw.o sw.dump s s.dump


