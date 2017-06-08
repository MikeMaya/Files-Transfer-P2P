//Compilar con g++ plantilla.cpp -std=c++11
#include "PaqueteDatagrama.h"
#include "SocketDatagrama.h"
#include <bits/stdc++.h>
#include <unordered_map>
using namespace std;

unordered_map<string, vector<string> > Archivos;
queue<string> Pendientes;
vector<string> IPS;

void* detectarServicios(void*);
void* eliminar(void*);
void* escuchar(void*);
void* manejoDirectorios(void*);


int main(){
	
	pthread_t th[4];

    pthread_create(&th[0], NULL, detectarServicios, NULL); 
    pthread_create(&th[1], NULL, eliminar, NULL);
	pthread_create(&th[2], NULL, escuchar, NULL); 
    pthread_create(&th[3], NULL, manejoDirectorios, NULL);

    pthread_join(th[0], NULL);
    pthread_join(th[1], NULL);
    pthread_join(th[2], NULL);
    pthread_join(th[3], NULL);
	
	return 0;
}

