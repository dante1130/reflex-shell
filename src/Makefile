#makefile


testtoken: testtoken.o token.o command.o
	gcc testtoken.o token.o command.o -o testtoken.out

token.o: token.c token.h
	gcc -c token.c

testtoken.o: testtoken.c token.h command.h
	gcc -c testtoken.c

command.o: command.c command.h
	gcc -c command.c

clean:
	rm *.o
