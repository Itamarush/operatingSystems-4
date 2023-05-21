all: server_reactor

server_reactor: server_reactor.o st_reactor.so
	gcc -g -o server_reactor server_reactor.o ./st_reactor.so -pthread

st_reactor.so: st_reactor.o
	gcc -g -shared -o st_reactor.so st_reactor.o -pthread

st_reactor.o: st_reactor.c st_reactor.h
	gcc -g -fPIC -c st_reactor.c st_reactor.h

server_reactor.o: server_reactor.c server_reactor.h
	gcc -g -c server_reactor.c

clean:
	rm -f *.o *.so *.gch server_reactor
