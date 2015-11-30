CPPFLAGS=-std=c++0x -pthread
LDLIBS=-lpthread

DEPS = server.h device.h clientregistry.h
OBJ = device.o server.o clientregistry.o main.o


$%.o: %.c $(DEPS)
	g++ $(CPPFLAGS) -c -o $@ $< 

server: $(OBJ)
	g++ $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean

clean:
	rm -f server *.o