include make.defs


OBJECTS = Sec.o



TARGET_LIST = Sec s.dump s.txt

all : $(TARGET_LIST)


Sec: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)


Sec.o: Sec.c
	$(CC) $(C_FLAGS) Sec.c
	
s.dump: Sec
	objdump -Dr Sec > $@

s.txt: Sec
	readelf -a Sec > $@

	
clean:
	rm -rf $(OBJECTS) $(TARGET_LIST)
