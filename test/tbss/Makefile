all:
	gcc -mno-abicalls -G 0 -fno-pic -c -o tbss.o tbss.c
	ld -EL -N -G 0 -Ttext 0x81000000 -e tbss -o tbss.elf tbss.o
	objcopy -R .reginfo tbss.elf tbss
	objcopy -O binary tbss tbss.bin
	objcopy -O binary --set-section-flags .bss=alloc,load,contents tbss tbss.bin2
	objcopy -R .reginfo -O binary --set-section-flags .bss=alloc,load,contents tbss.elf tbss.bin3
#	ld -EL -N -G 0 -Ttext 0x81000000 -e tbss --oformat binary -o t.bin tbss.o

clean:
	rm -rf tbss.o tbss tbss.bin tbss.elf tbss.bin*
