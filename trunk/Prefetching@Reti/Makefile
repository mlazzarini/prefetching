GCCFLAGS= -Wall -Wno-unused-function -ansi -pedantic -ggdb
LINKERFLAGS=-lpthread -lm

all: Server1.exe Server2.exe Client1.exe Client2.exe Proxy.exe
	

Client1.exe: Client1.o Util.o
	gcc -o Client1.exe ${GCCFLAGS} ${LINKERFLAGS} Client1.o Util.o 

Client1.o: Client.c Util.h 
	gcc -c ${GCCFLAGS} -o Client1.o Client.c

Client2.exe: Client2.o Util.o
	gcc -o Client2.exe ${GCCFLAGS} ${LINKERFLAGS} Client2.o Util.o 

Client2.o: Client.c Util.h 
	gcc -c ${GCCFLAGS} -DCLIENT2 -o Client2.o Client.c

Server1.exe: Server1.o Util.o
	gcc -o Server1.exe ${GCCFLAGS} ${LINKERFLAGS} Server1.o Util.o 

Server1.o: Server.c Util.h 
	gcc -c ${GCCFLAGS} -o Server1.o Server.c

Server2.exe: Server2.o Util.o
	gcc -o Server2.exe ${GCCFLAGS} ${LINKERFLAGS} Server2.o Util.o 

Server2.o: Server.c Util.h 
	gcc -c ${GCCFLAGS} -DSOLORANGE -o Server2.o Server.c

Proxy.exe: Proxy.o Util.o Parser.o Request.o IOUtil.o Cache.o
	gcc -o Proxy.exe ${GCCFLAGS} ${LINKERFLAGS} Proxy.o Util.o Parser.o Request.o IOUtil.o Cache.o

Proxy.o: Proxy.c Util.h Request.h
	gcc -c ${GCCFLAGS} Proxy.c
	
Util.o: Util.c Util.h 
	gcc -c ${GCCFLAGS} Util.c
	
Parser.o: Parser.c Parser.h
	gcc -c ${GCCFLAGS} Parser.c
	
Request.o: Request.c Request.h
	gcc -c ${GCCFLAGS} Request.c
	
IOUtil.o: IOUtil.c IOUtil.h
	gcc -c ${GCCFLAGS} IOUtil.c
	
Cache.o: Cache.c Cache.h
	gcc -c ${GCCFLAGS} Cache.c
	

clean:	
	rm -f core* *.stackdump
	rm -f *.exe
	rm -f *.o

cleanall:	clean
	rm -f *~

