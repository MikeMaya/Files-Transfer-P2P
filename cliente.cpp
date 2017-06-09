//Compilar con g++ plantilla.cpp -std=c++11
#include "PaqueteDatagrama.h"
#include "SocketDatagrama.h"
#include <bits/stdc++.h>
#include <unordered_map>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#define BUF_SIZE 1024
using namespace std;

//Estructuras Globales
unordered_map<string, vector<string> > Archivos;
queue<string> Pendientes;
vector<string> IPS;

/*Códigos de error */
#define OK              0
#define E_BAD_OPCODE    -1
#define E_BAD_PARAM     -2
#define E_IO            -3

//Parametros globales
string directorio="/home/Carpeta/";
string basura="/home/Basura/";
string direccionBroadcast="10.0.0.255";
int puertoServicios=7744;
int puertoEliminar=7745;
int puertoEscucha=7746;
int puertoArchivos=7747;

//Sockets compartidos
SocketDatagrama socketServicio(puertoServicios);


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
    uint32_t result;
};

//Hilos
void* detectarServicios(void*);
void* broadcast(void*);
void* eliminar(void*);
void* escuchar(void*);
void* manejoDirectorios(void*);

int main(){
	
    if(socketServicio.setBroadcast() < 0){
        return 0;
    }

	pthread_t th[5];

    pthread_create(&th[0], NULL, detectarServicios, NULL); 
    pthread_create(&th[1], NULL, broadcast, NULL); 
    //pthread_create(&th[2], NULL, eliminar, NULL);
	pthread_create(&th[3], NULL, escuchar, NULL); 
    pthread_create(&th[4], NULL, manejoDirectorios, NULL);

    pthread_join(th[0], NULL);
    pthread_join(th[1], NULL);
    //pthread_join(th[2], NULL);
    pthread_join(th[3], NULL);
    pthread_join(th[4], NULL);
	
	return 0;
}

bool existe(string& a, vector<string>& v){
    for(string b: v)
        if(a==b) return true;
    return false;
}

void* escuchar(void*){
    SocketDatagrama s(puertoEscucha);
    Peticion pet;
    Respuesta res;
    vector<string> v;
    int fileDescriptor;
    char *filename;

    while(1){
        PaqueteDatagrama paqpet(sizeof(Peticion));
        PaqueteDatagrama paqres(sizeof(Respuesta));
        bzero((char *)&pet, sizeof(Peticion));
        bzero((char *)&res, sizeof(Respuesta));

        s.recibe(paqpet);
        memcpy((char *)&pet, paqpet.obtieneDatos(), sizeof(Peticion));
        string ipRemota(paqpet.obtieneDireccion());
        filename = new char[directorio.size() + strlen(pet.nombre) + 1];
        strcpy(filename, directorio.c_str());
        strcat(filename, pet.nombre);

        switch(pet.codigo){
            case 1: // Anuncio archivo
                if( Archivos.count(pet.nombre) == 0 ){
                    cout << "Verificando el archivo \"" << pet.nombre << "\"" << endl;
                    cout << "No lo tenemos\n";
                    Pendientes.push(pet.nombre);
                    Archivos.emplace(string(pet.nombre), vector<string>());
                }

                v = Archivos[pet.nombre];
                if( !existe(ipRemota, v) ){
                    cout << "Primera vez  de la ip " << ipRemota << endl;
                    v.push_back(ipRemota);
                    Archivos[pet.nombre] = v;
                }
                //cout << endl;
                break;

            case 2: // Peticion de un archivo
                cout << "Buscando el archivo " << filename << endl;

                fileDescriptor = open(filename, O_RDONLY);
                if( fileDescriptor == -1 )
                    res.result = E_IO;
                else{
                    lseek(fileDescriptor, pet.offset, SEEK_SET);
                    res.count = read(fileDescriptor, res.data, BUF_SIZE);
                    res.result = OK;
                    close(fileDescriptor);
                }
                paqres.inicializaDatos((char *)&res);
                paqres.inicializaIp(paqpet.obtieneDireccion());
                paqres.inicializaPuerto(paqpet.obtienePuerto());
                s.envia(paqres);
                break;

            case 3: // Eliminar archivo
                // cout << "Eliminando el archivo " << filename << endl;

                if( access(filename, F_OK) != -1 ){
                    if( remove(filename) == -1 ){
                        cout << "Error removiendo el archivo \"" << pet.nombre << "\"" << endl;
                        break;
                    }
                }
                
                Archivos.erase(pet.nombre);

                fileDescriptor = 1;
                paqres.inicializaDatos((char *)&fileDescriptor);
                paqres.inicializaIp(paqpet.obtieneDireccion());
                paqres.inicializaPuerto(paqpet.obtienePuerto());
                s.envia(paqres);
                break;

            default:
                cout << "Codigo no reconocido: " << pet.codigo << endl;
                cout << "   Direccion: " << paqpet.obtieneDireccion();
                cout << " : " << paqpet.obtienePuerto() << endl;
        }
    }
}

void* broadcast(void*){
    int num[2];
    num[1]=5;
    num[0]=3;
    while(1){
        //cout<<"Enviando mi direccion\n";
        PaqueteDatagrama  p((char *)num, 2*sizeof(int), (char*) direccionBroadcast.c_str(), puertoServicios); 
        socketServicio.envia(p);
        sleep(10);
    }
}

void* detectarServicios(void*){
    //cout<<"Esperando servicios disponibles\n";
    PaqueteDatagrama p2(sizeof(int));
    while(1){   
        socketServicio.recibe(p2);
        //cout<<"Servico disponible en: "<<p2.obtieneDireccion()<<'\n';
        IPS.push_back(string(p2.obtieneDireccion()));
        //sleep(10);
    }   
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
            cout<<"Encontrado "<<nuevo<<'\n';
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
                cout<<"Archivo nuevo "<<nuevo<<'\n';
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
        //cout<<"Anunciando "<<pet.nombre<<'\n';
        socket.envia(p);
    }
    return;
}

string siguienteValido(vector<string>& direcciones, int& i, int& largo){
    int inc=0;  
    while(inc < largo){
        for(string ip: IPS){
            cout<<"IP "<<ip<<" - "<<direcciones[(i+inc)%largo]<<endl;
            if(ip == direcciones[(i+inc)%largo])  return ip;        
        }
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
        direcciones= Archivos[actual];
        noIPS=direcciones.size();
        cout<<"Pidiendo "<<actual<<endl;
        //Creamos el archivo
        ofstream archivo;
        archivo.open (directorio+actual, ios::out | ios::binary);

        while(ban){

            pet.offset=offset;
            dir= siguienteValido(direcciones, i, noIPS);
            cout<<offset<<endl;
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
    //cout<<"Terminando pendientes\n";
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