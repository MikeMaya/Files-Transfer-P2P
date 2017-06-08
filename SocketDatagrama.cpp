#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include "SocketDatagrama.h"
#include "PaqueteDatagrama.h"

using namespace std;

SocketDatagrama::SocketDatagrama(int puerto)
{
	s = socket(AF_INET, SOCK_DGRAM, 0);
	bzero((char *)&direccionLocal, sizeof(direccionLocal));
	bzero((char *)&direccionForanea, sizeof(direccionForanea));

	direccionLocal.sin_family = AF_INET;
	direccionLocal.sin_addr.s_addr = INADDR_ANY;
	direccionLocal.sin_port = htons(puerto);
	bind(s, (struct sockaddr *)&direccionLocal, sizeof(direccionLocal));
	timeout = false;
}

SocketDatagrama::~SocketDatagrama()//Recibe un paquete tipo datagrama proveniente de este socket
{
	close(s);
//	cout << "Se elimino el socket" << endl;

}
	
int SocketDatagrama::recibe(PaqueteDatagrama & p) //Envía un paquete tipo datagrama desde este socket
{
	socklen_t len = sizeof(direccionForanea);
	unsigned char direccion[16];

	int retorno = recvfrom(s, p.obtieneDatos(), p.obtieneLongitud(), 0, (struct sockaddr*)&direccionForanea, &len);

	p.inicializaPuerto(ntohs(direccionForanea.sin_port));
	p.inicializaIp(inet_ntoa(direccionForanea.sin_addr));

	//cout << "Mensaje recibido desde: " << inet_ntoa(direccionForanea.sin_addr) << endl;

	return retorno;
}

int SocketDatagrama::envia(PaqueteDatagrama & p)
{
	direccionForanea.sin_family = AF_INET;
	direccionForanea.sin_addr.s_addr = inet_addr(p.obtieneDireccion());
	direccionForanea.sin_port = htons(p.obtienePuerto());

	int retorno = sendto(s, p.obtieneDatos(), p.obtieneLongitud(), 0, (struct sockaddr*)&direccionForanea, sizeof(direccionForanea));
	return retorno;
}

void SocketDatagrama::setTimeout(time_t seg, suseconds_t microseg)
{
	timeout = true;
	tiempofuera.tv_sec = seg;
	tiempofuera.tv_usec = microseg;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tiempofuera, sizeof(tiempofuera));
}

void SocketDatagrama::unsetTimeout()
{
	timeout = false;
	tiempofuera.tv_sec = 0;
	tiempofuera.tv_usec = 0;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tiempofuera, sizeof(tiempofuera));

}

int SocketDatagrama::recibeTimeout(PaqueteDatagrama & p)
{
	socklen_t len = sizeof(direccionForanea);
	unsigned char direccion[16];
	struct timeval t0, t1, res;
	gettimeofday(&t0,NULL);
	int retorno = recvfrom(s, p.obtieneDatos(), p.obtieneLongitud(), 0, (struct sockaddr*)&direccionForanea, &len);
	if (retorno < 0) 
	{
		if (errno != EWOULDBLOCK)
			cout << "Error en recvfrom" << endl;
	}
	else 
	{
		gettimeofday(&t1,NULL);
		timersub(&t1,&t0,&res);
		p.inicializaPuerto(ntohs(direccionForanea.sin_port));
		p.inicializaIp(inet_ntoa(direccionForanea.sin_addr));	
		//cout << "Mensaje recibido desde: " << inet_ntoa(direccionForanea.sin_addr) << endl;
		//cout << "Tiempo de respuesta: " << res.tv_sec << " segundos " << res.tv_usec << " microsegundos" << endl;
	}
	
	return retorno;
}
int SocketDatagrama::setBroadcast(){
	int yes=1, retorno; 
	retorno = setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int));
	return retorno;

}

