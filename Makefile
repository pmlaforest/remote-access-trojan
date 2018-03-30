GCC=g++
OPT= -std=c++11
DEST=/home/pmlaforest/Documents/projets/remote-access-trojan/bin

all: injecteurClient injecteurServer RATClient RATServer

injecteurClient: comClientServer.o injecteurClient.o
	$(GCC) $(OPT) $(DEST)/injecteurClient.o $(DEST)/comClientServer.o -o $(DEST)/injecteurClient

injecteurClient.o: injecteurClient.cpp comClientServer.h
	$(GCC) $(OPT) -c injecteurClient.cpp -o $(DEST)/injecteurClient.o

injecteurServer: comClientServer.o injecteurServer.o
	$(GCC) $(OPT) $(DEST)/injecteurServer.o $(DEST)/comClientServer.o -o $(DEST)/injecteurServer

injecteurServer.o: injecteurServer.cpp comClientServer.h
	$(GCC) $(OPT) -c injecteurServer.cpp -o $(DEST)/injecteurServer.o

comClientServer.o: comClientServer.cpp comClientServer.h
	$(GCC) $(OPT) -c comClientServer.cpp -o $(DEST)/comClientServer.o

RATServer: RATServer.o RATFonctions.o comClientServer.o
	$(GCC) $(OPT) $(DEST)/RATServer.o $(DEST)/RATFonctions.o $(DEST)/comClientServer.o -o $(DEST)/RATServer

RATServer.o: RATServer.cpp
	$(GCC) $(OPT) -c RATServer.cpp -o $(DEST)/RATServer.o

RATClient: RATClient.o RATFonctions.o comClientServer.o
	$(GCC) $(OPT) $(DEST)/RATClient.o $(DEST)/RATFonctions.o $(DEST)/comClientServer.o -o $(DEST)/RATClient

RATClient.o: RATClient.cpp
	$(GCC) $(OPT) -c RATClient.cpp -o $(DEST)/RATClient.o

RATFonctions.o: RATFonctions.cpp RATFonctions.h
	$(GCC) $(OPT) -c RATFonctions.cpp -o $(DEST)/RATFonctions.o

clean:
	rm /home/pmlaforest/Documents/projets/remote-access-trojan/bin/*
