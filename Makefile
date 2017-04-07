TARGETA = tcpserver
OBJECTSA = tcpserver.o
TARGETB = tcpklijent
OBJECTSB = tcpklijent.o
COMMON = mrepro.o

all: $(TARGETA) $(TARGETB)

$(TARGETA): $(OBJECTSA) $(COMMON)
	$(CC) -o $(TARGETA) $(OBJECTSA) $(COMMON)

$(TARGETB): $(OBJECTSB) $(COMMON)
	$(CC) -o $(TARGETB) $(OBJECTSB) $(COMMON)

clean:
	rm -f $(TARGETA) $(TARGETB) $(OBJECTSA) $(OBJECTSB) $(COMMON)
