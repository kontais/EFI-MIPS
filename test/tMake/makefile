all:a.c ;	@echo xxxx


test:
	-slslls

b:b/b.o


#b/b.o:b/$(patsubst %.o, %.c, $@)
b/b.o:b/$(patsubst %.o, %.c, b/b.o)
	@echo $@, $(@F:.o=.c), $^

clean:
	@echo clean
