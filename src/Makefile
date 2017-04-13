all: public.o main.o
	gcc public.o main.o -o main
public:
	gcc -c public.c
main:
	gcc -c main.c
clean:
	rm -f *.o