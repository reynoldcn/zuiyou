all: public.o hash.o main.o
	gcc public.o hash.o main.o -o main
public:
	gcc -c public.c
hash:
	gcc -c hash.c
main:
	gcc -c main.c
clean:
	rm -f *.o