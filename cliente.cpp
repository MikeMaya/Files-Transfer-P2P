//Compilar con g++ plantilla.cpp -std=c++11
#include "PaqueteDatagrama.h"
#include "SocketDatagrama.h"
#include <bits/stdc++.h>
#include <unordered_map>
#include <dirent.h>
#define BUF_SIZE 1024
using namespace std;

//Estructuras Globales
unordered_map<string, vector<string> > Archivos;
queue<string> Pendientes;
vector<string> IPS;

//Parametros globales
string directorio="/home/Carpeta/";
string basura="/home/Basura/";
string direccionBroadcast="192.168.0.255";
int puertoServicios=7744;
int puertoEliminar=7745;
int puertoEscucha=7746;
int puertoArchivos=7747;


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

    //pthread_create(&th[0], NULL, detectarServicios, NULL); 
    //pthread_create(&th[1], NULL, eliminar, NULL);
	//pthread_create(&th[2], NULL, escuchar, NULL); 
    pthread_create(&th[3], NULL, manejoDirectorios, NULL);

    //pthread_join(th[0], NULL);
    //pthread_join(th[1], NULL);
    //pthread_join(th[2], NULL);
    pthread_join(th[3], NULL);
	
	return 0;
}

bool existe(string& a, vector<string>& v){
    for(string b: v)
        if(a==b) return true;
    return false;
}

vector<string> arranque(){
    vector<string> archivos;
    string nuevo;
    DIR *dir;
    struct dirent *d;
    FILE *file;
    //Leer de directorio
    dir= opendir(directorio.c_str());
    while((d = readdir(dir))!= NULL){
        if( d->d_type == DT_REG){
            nuevo = string(d->d_name);
            archivos.push_back(nuevo);
        }
    }
    closedir(dir);
    //Meter en la tabla    
    for(string a: archivos){
        Archivos.emplace(a, vector<string>());
    }
    //Regresar los archivos
    return archivos;
}

void verificarCambios(vector<string>&archivos ){
    string nuevo;
    DIR *dir;
    struct dirent *d;
    FILE *file;
    //Leer de directorio
    dir= opendir(directorio.c_str());
    while((d = readdir(dir))!= NULL){
        if( d->d_type == DT_REG){
            nuevo = string(d->d_name);
            if(!existe(nuevo, archivos)){
                archivos.push_back(nuevo);
                Archivos.emplace(nuevo, vector<string>());
            }
        }
    }
    closedir(dir);
    //Meter en la tabla
}

void anunciarPropios(vector<string>& archivos, SocketDatagrama& socket){
    Peticion pet;
    pet.codigo=1;
    if(socket.setBroadcast() < 0) return;
    for(string arch: archivos){
        strcpy(pet.nombre, arch.c_str());
        PaqueteDatagrama p((char *)&pet, sizeof(Peticion),(char*) direccionBroadcast.c_str(), puertoEscucha); 
        socket.envia(p);
    }
    return;
}

string siguienteValido(vector<string>& direcciones, int& i, int& largo){
    int inc=0;
    while(inc < largo){
        for(string ip: IPS)
            if(ip == direcciones[(i+inc)%largo])  return ip;        
        inc++;
    }
    return "";
}

void pedirFaltantes(SocketDatagrama& socket){
    string actual;
    Peticion pet;
    Respuesta res;
    vector<string> direcciones;
    string dir;

    pet.codigo=2;
    int i=0, e;
    uint32_t offset=0;
    bool ban=true;

    socket.setTimeout(0,500000);
    int noIPS= direcciones.size();

    while(!Pendientes.empty()){
        //Inicializamos el archivo a peir
        actual= Pendientes.front();
        Pendientes.pop();
        ban=true;
        offset=0;
        i=0;
        //Creamos el archivo
        ofstream archivo;
        archivo.open (directorio+actual, ios::out | ios::binary);

        while(ban){

            pet.offset=offset;
            dir= siguienteValido(direcciones, i, noIPS);
            //No exite direccion Conectada que tenga ese archivo
            if(dir.empty()){
                cout<<"NO ES POSIBLE ENCONTRAR DIRECCION PARA: "<<actual<<"\n";
                //Esto deberia de mostrarse una excepcion o asi... no se me ha ocurrido nada
                //Pendientes.push(actual);
                break; 
            }

            PaqueteDatagrama p((char *)&pet, sizeof(Peticion),(char*) dir.c_str(), puertoEscucha); 
            PaqueteDatagrama p2(sizeof(Respuesta));
            
            socket.envia(p);
            e=socket.recibeTimeout(p2);
            
            if(e>=0){
                memcpy(&res, p2.obtieneDatos(),sizeof(Respuesta));    
                offset+=res.count;                
                archivo.write(res.data, res.count);
                if(res.count< BUF_SIZE){
                    archivo.close();
                    ban=false;
                }
            }
            i= (i+1)%noIPS;
        }
    }    
    return;
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