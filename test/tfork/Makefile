all: execvp  m  twaitpid

m : m.c
	gcc -o $@  $^
execvp : execvp.c
	gcc -o $@  $^
twaitpid : twaitpid.c
	gcc -o $@  $^

clean:
	rm -rf execvp  m  twaitpid
