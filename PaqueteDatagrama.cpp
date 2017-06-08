#include "PaqueteDatagrama.h"
#include "SocketDatagrama.h"
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

PaqueteDatagrama::PaqueteDatagrama(char * dat, unsigned int longi, char *dirip, int port)
{
	datos = new char [longi+1];
	memcpy(datos, dat, longi);
	longitud = longi;
	memcpy(ip, dirip, 16);
	puerto = port;
}

PaqueteDatagrama::PaqueteDatagrama(unsigned int longi)
{
	datos = new char [longi+1];
	longitud = longi;
}

PaqueteDatagrama::~PaqueteDatagrama()
{
	delete[] datos;
//	cout << "Se liberó la memoria" << endl;
}

char * PaqueteDatagrama::obtieneDireccion()
{
	return ip;
}

unsigned int PaqueteDatagrama::obtieneLongitud()
{
	return longitud;
}

int PaqueteDatagrama::obtienePuerto()
{
	return puerto;
}

char * PaqueteDatagrama::obtieneDatos()
{
	return datos;
}

void PaqueteDatagrama::inicializaPuerto(int port)
{
	puerto = port;
}

void PaqueteDatagrama::inicializaIp(char * dirip)
{
	memcpy(ip, dirip, 16);
}

void PaqueteDatagrama::inicializaDatos(char * dat)
{
	memcpy(datos, dat, longitud);
}

