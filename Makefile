cliente: cliente.cpp SocketDatagrama.o
	g++ cliente.cpp SocketDatagrama.o PaqueteDatagrama.o -pthread -o cliente

SocketDatagrama.o: SocketDatagrama.cpp PaqueteDatagrama.o
	g++ SocketDatagrama.cpp -c

PaqueteDatagrama.o: PaqueteDatagrama.cpp
	g++ PaqueteDatagrama.cpp -c

clean:
	rm *.o cliente
