LD = ld
LD_FLAGS = -EL -N
#LD_FLAGS =
CC = gcc
READELF = readelf
OBJDUMP = objdump
LOADADDR = 0x00100000
OD = od
all: test

test: clean test.o
	$(LD) $(LD_FLAGS) -r -e fa -o test test.o
	$(READELF) -a test > test.readelf
	$(OBJDUMP) -Dtr test > test.dump
	$(OD) -A x -t x4 -v test > test.hex
test.elf: clean test.o
	$(LD) $(LD_FLAGS) -r -e fa -o test.rel test.o
	$(LD) $(LD_FLAGS) -Ttext $(LOADADDR) -e fa -o test test.rel
	$(READELF) -a test > test.readelf
	$(OBJDUMP) -Dtr test > test.dump
	$(OD) -A x -t x4 -v test > test.hex
test.o: test.c
	$(CC) -c -o test.o test.c
clean:
	$(RM) test test.o test.readelf test.dump test.hex
