all: a.c a.s
	cpp -E -dM -march=loongson2e -mtune=loongson2e -mabi=32 -mips3 a.c > mc.txt
	cpp -E -dM -mabi=n32 -mips3 a.c > mcn32.txt
	cpp -E -dM -march=loongson2e -mtune=loongson2e -mabi=32 -mips3 a.s > ms.txt
	cpp -E -dM -march=loongson2f -mtune=loongson2f -mabi=64 a.c > mc64.txt
	cpp -E -dM -march=loongson2f -mtune=loongson2f -mabi=64 a.s > ms64.txt
clean:
	rm -rf *.txt
