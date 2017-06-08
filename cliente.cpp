//Compilar con g++ plantilla.cpp -std=c++11
#include <bits/stdc++.h>
#include <unordered_map>
#define BUF_SIZE 1024
using namespace std;

//Estructuras Globales
unordered_map<string, vector<string> > Archivos;
queue<string> Pendientes;
vector<string> IPS;

//Parametros globales
string directorio="";
string basura="";
string direccionBroadcast="192.168.0.255";
puertoServicios=7744;
puertoEliminar=7745;
puertoEscucha=7746;
puertoArchivos=7747;


//Definir estructura de mensaje
//Estructura usada para realizar una peticion
struct Peticion{
    int codigo;
    char nombre[BUF_SIZE];
    uint32_t offset;
};
//Estructura usada para realizar la respuesta de archivos
struct Respuesta{
    char data[BUF_SIZE]; 
    uint32_t count;
};


//Hilos
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

void* manejoDirectorios(void* args){
    SocketDatagrama s(puertoArchivos);
    //Cargar archivos iniciales
    vector<string> archivos= arranque();
    while(1){
        anunciarPropios(archivos, s);
        pedirFaltantes(s);
        verificarCambios(archivos);
    }
}

vector<string> arranque(){
    vector<string> archivos;
    //Leer de directorio

    //Meter en la tabla    
    return archivos;
}

void verificarCambios(vector<string>&archivos ){
    //Leer de directorio
    
    //Meter en la tabla
    return archivos;
}

void anunciarPropios(vector<string>& archivos, SocketDatagrama& socket){
    Peticion pet;
    pet.codigo=1;
    if(socket.setBroadcast() < 0) return;
    for(string arch: archivos){
        pet.nombre=arch;
        PaqueteDatagrama p((char *)&pet, sizeof(Peticion), direccionBroadcast, puertoEscucha); 
        s.envia(p);
    }
    return;
}

string siguienteValido(vector<string>& direcciones, int& i, int& largo){
    int inc=0;
    while(inc < largo){
        for(string ip: IPS)
            if(ip==direcciones[(i+inc)%largo])  return ip;        
        inc++;
    }
    return NULL;
}

void pedirFaltantes(SocketDatagrama& socket){
    string actual;
    Peticion pet;
    Respuesta res;
    vector<string> direcciones;
    string dir;

    pet.codigo=2;
    int i=0;
    uint32_t offset=0;
    bool ban=true;

    s.setTimeout(0,500000);
    int noIPS= direcciones.size();

    while(!Pendientes.empty()){
        //Inicializamos el archivo a peir
        actual= Pendientes.pop();
        ban=true;
        offset=0;
        i=0;
        //Creamos el archivo
        ofstream archivo;
        archivo.open (directorio+actual, ios::out | ios::binary);

        while(ban){

            pet.offset=offset;
            dir= siguienteValido(direcciones, i);
            //No exite direccion Conectada que tenga ese archivo
            if(dir==NULL){
                cout<<"NO ES POSIBLE ENCONTRAR DIRECCION PARA: "<<actual<<"\n";
                //Esto deberia de mostrarse una excepcion o asi... no se me ha ocurrido nada
                //Pendientes.push(actual);
                break; 
            }

            PaqueteDatagrama p((char *)&pet, sizeof(Peticion), dir, atoi(puertoEscucha)); 
            PaqueteDatagrama p2(sizeof(Respuesta));
            
            socket.envia(p);
            e=s.recibeTimeout(p2);
            
            if(e>=0){
                memcpy(&res, p2.obtieneDatos(),sizeof(Respuesta));    
                offset+=res.count;                
                archivo.write(res.data, res.count);
                if(respuesta.count< BUF_SIZE){
                    archivo.close();
                    ban=false;
                }
            }
            i= (i+1)%noIPS;
        }
    }    
    return;
}
