all: a.c b.c c.c
	gcc -c -o a.o a.c
	gcc -c -o b.o b.c
	ar r ab.a a.o b.o
	gcc -c -o c.o c.c
	ar r abc.a c.o ab.a
	ar -t abc.a
	gcc -o m1 main.c abc.a ab.a # 可以
	objdump -d m1 > m1.txt
	gcc -o m2 main.c c.o ab.a   # 可以
	objdump -d m2 > m2.txt
#	gcc -o m3 main.c abc.a      # 找不到fa,fb

clean:
	rm -rf *.o *.a m? *.txt

