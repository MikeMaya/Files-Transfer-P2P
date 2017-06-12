clienteC: cliente.cpp SocketDatagrama.o
	g++ cliente.cpp SocketDatagrama.o PaqueteDatagrama.o -pthread -o cliente -std=c++11

all: clienteC clienteJ

clienteJ: Peticion.java Respuesta.java servicio.java cliente.java
	javac Peticion.java Respuesta.java servicio.java cliente.java

SocketDatagrama.o: SocketDatagrama.cpp PaqueteDatagrama.o
	g++ SocketDatagrama.cpp -c

PaqueteDatagrama.o: PaqueteDatagrama.cpp
	g++ PaqueteDatagrama.cpp -c

clean:
	rm -f *.o *.class cliente
