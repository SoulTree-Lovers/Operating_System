all: fork cat 
 
fork: fork.c
	gcc -o fork fork.c
	
exec: exec.c
	gcc -o exec exec.c

cat: cat.c
	gcc -o cat cat.c

clean:
	rm fork exec
