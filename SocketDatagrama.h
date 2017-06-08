#ifndef SocketDatagrama_H_
#define SocketDatagrama_H
#include "PaqueteDatagrama.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

class SocketDatagrama{
public:
	SocketDatagrama(int);
	~SocketDatagrama();
	//Recibe un paquete tipo datagrama proveniente de este socket
	int recibe(PaqueteDatagrama & p);
	//Envía un paquete tipo datagrama desde este socket
	int envia(PaqueteDatagrama & p);
	void setTimeout(time_t seg, suseconds_t microseg);
	void unsetTimeout();
	int recibeTimeout(PaqueteDatagrama & p);
	int setBroadcast();
private:
	struct sockaddr_in direccionLocal;
	struct sockaddr_in direccionForanea;
	struct timeval tiempofuera;
	int s; //ID socket
	bool timeout;
};
#endif
