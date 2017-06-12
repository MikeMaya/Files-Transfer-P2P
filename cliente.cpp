//Compilar con g++ plantilla.cpp -std=c++11
#include "PaqueteDatagrama.h"
#include "SocketDatagrama.h"
#include <bits/stdc++.h>
#include <unordered_map>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#define BUF_SIZE 5024
#define NAME_SIZE 1024
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
string direccionBroadcast="10.100.95.255";
string eliminando="";
int puertoServicios=7744;
int puertoEliminar=7745;
int puertoEscucha=7746;
int puertoArchivos=7747;
int noFallos=7;

//Sockets compartidos
SocketDatagrama socketServicio(puertoServicios);


//Estructura usada para realizar una peticion
struct Peticion{
    int codigo;
    uint32_t offset;
    char nombre[NAME_SIZE];
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
void* escuchar(void*);
void* manejoDirectorios(void*);

int main(int argc, const char *argv[]){

    if(argc != 3){
        printf("%s\n", "Uso: ./cliente carpeta_nodo carpeta_basura");
        exit(1);
    }

    directorio = string(argv[1]);
    if( directorio[directorio.length() - 1] != '/' )
        directorio += "/";

    basura = string(argv[2]);
    if( basura[basura.length() - 1] != '/' )
        basura += "/";
    
    if(socketServicio.setBroadcast() < 0){
        return 0;
    }

	pthread_t th[5];

    pthread_create(&th[0], NULL, detectarServicios, NULL); 
    pthread_create(&th[1], NULL, broadcast, NULL); 
	pthread_create(&th[3], NULL, escuchar, NULL); 
    pthread_create(&th[4], NULL, manejoDirectorios, NULL);

    pthread_join(th[0], NULL);
    pthread_join(th[1], NULL);
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
        PaqueteDatagrama paqcon(sizeof(int));
        bzero((char *)&pet, sizeof(Peticion));
        bzero((char *)&res, sizeof(Respuesta));

        s.recibe(paqpet);
        memcpy((char *)&pet, paqpet.obtieneDatos(), sizeof(Peticion));
        string ipRemota(paqpet.obtieneDireccion());
        filename = new char[directorio.size() + strlen(pet.nombre) + 1];
        strcpy(filename, directorio.c_str());
        strcat(filename, pet.nombre);
        cout<<"Paquete "<<paqpet.obtieneDireccion()<<" - "<<pet.codigo<<" - "<<pet.nombre<<endl;
        switch(pet.codigo){
            case 1: // Anuncio archivo
                cout<<"trabajando en: " <<eliminando<<endl;
                if( string(pet.nombre) == eliminando) break;
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
                cout << "Eliminando el archivo " << filename << endl;

                if( access(filename, F_OK) != -1 ){
                    if( remove(filename) == -1 ){
                        cout << "Error removiendo el archivo \"" << pet.nombre << "\"" << endl;
                        break;
                    }
                }
                
                Archivos.erase(string(pet.nombre));

                fileDescriptor = 1;
                paqcon.inicializaDatos((char *)&fileDescriptor);
                paqcon.inicializaIp(paqpet.obtieneDireccion());
                paqcon.inicializaPuerto(paqpet.obtienePuerto());
                s.envia(paqcon);
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
        sleep(30);
        //IPS.clear();
    }
}

void* detectarServicios(void*){
    //cout<<"Esperando servicios disponibles\n";
    PaqueteDatagrama p2(sizeof(int));
    while(1){   
        socketServicio.recibe(p2);
        //cout<<"Servico disponible en: "<<p2.obtieneDireccion()<<'\n';
        string a(p2.obtieneDireccion());
        if(!existe(a, IPS))
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
            //cout<<"Encontrado "<<nuevo<<'\n';
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

void anunciarPropios(SocketDatagrama& socket){
    string nuevo;
    DIR *dir;
    struct dirent *d;
    FILE *file;
    Peticion pet;
    pet.codigo=1;
    if(socket.setBroadcast() < 0) return;
    //Leer de directorio
    dir= opendir(directorio.c_str());
    while((d = readdir(dir))!= NULL){
        if( d->d_type == DT_REG){
            nuevo = string(d->d_name);
            if(Archivos.count(nuevo)==0){
                //cout<<"Archivo nuevo "<<nuevo<<'\n';
                Archivos.emplace(nuevo, vector<string>());
            }
            strcpy(pet.nombre, nuevo.c_str());
            PaqueteDatagrama p((char *)&pet, sizeof(Peticion),(char*) direccionBroadcast.c_str(), puertoEscucha); 
            socket.envia(p);            
        }
    }
    closedir(dir);
}

string siguienteValido(vector<string>& direcciones, int& i, int& largo){
    int inc=0;  
    while(inc < largo){
        for(string ip: IPS){
            //cout<<"IP "<<ip<<" - "<<direcciones[(i+inc)%largo]<<endl;
            if(ip == direcciones[(i+inc)%largo])  return ip;        
        }
        inc++;
    }
    return "";
}

void desconectar(string ip){
    for(int i=0;i<IPS.size();i++){
        if(ip==IPS[i]){
            IPS.erase(IPS.begin()+i);
            break;
        }
    }
    return;
}

void pedirFaltantes(SocketDatagrama& socket){
    string actual;
    Peticion pet;
    Respuesta res;
    vector<string> direcciones;
    queue<string> auxiliar;
    string dir;

    pet.codigo=2;
    int i=0, e;
    uint32_t offset=0;
    bool ban=true;

    socket.setTimeout(5,0);
    int noIPS= direcciones.size();

    while(!Pendientes.empty()){
        //Inicializamos el archivo a peir
        actual= Pendientes.front();
        Pendientes.pop();
        ban=true;
        offset=0;
        i=0;
        strcpy(pet.nombre, actual.c_str());
        direcciones= Archivos[actual];
        noIPS=direcciones.size();
        vector<int> fallos(noIPS, 0);
        //cout<<"Pidiendo "<<actual<<endl;
        //Creamos el archivo
        ofstream archivo;
        archivo.open (directorio+actual, ios::out | ios::binary);
        //cout<<directorio+actual<<endl;
        while(ban){

            pet.offset=offset;
            dir= siguienteValido(direcciones, i, noIPS);
            //No exite direccion Conectada que tenga ese archivo
            if(dir.empty()){
                //cout<<"NO ES POSIBLE ENCONTRAR DIRECCION PARA: "<<actual<<"\n";
                archivo.close();
                if( remove((directorio+actual).c_str()) == -1 ){
                    cout << "Error al remover archivo \"" << pet.nombre << "\"" << endl;
                }
                auxiliar.push(actual);
                ban=false;
                break; 
            }

            cout<<offset<<" - "<<dir<<endl;
            PaqueteDatagrama p((char *)&pet, sizeof(Peticion),(char*) dir.c_str(), puertoEscucha); 
            PaqueteDatagrama p2(sizeof(Respuesta));
            
            socket.envia(p);
            e=socket.recibeTimeout(p2);
            
            if(e>=0){
                //cout<<"Respuesta recibida"<<endl;
                memcpy(&res, p2.obtieneDatos(),sizeof(Respuesta));    
                offset+=res.count;                
                fallos[i]=0;
                archivo.write(res.data, res.count);
                if(res.count< BUF_SIZE){
                    archivo.close();
                    ban=false;
                }
            }else {
                fallos[i]++;
                if(fallos[i]==noFallos){
                    desconectar(direcciones[i]);
                    fallos[i]=0;
                }
            }
            i= (i+1)%noIPS;
        }
    }
    while(!auxiliar.empty()){
        Pendientes.push(auxiliar.front());
        auxiliar.pop();
    }
    //cout<<"Terminando pendientes\n";
    return;
}

void eliminar(){
    SocketDatagrama s(puertoEliminar);

    Peticion pet;   
    PaqueteDatagrama res(sizeof(int));
    bool ban = true;    
    bool fallo = true;
    int r;
    string actual, completo;

    DIR *dir;
    struct dirent *d;

    s.setTimeout(1,0);
    //Leer de directorio
    dir= opendir(basura.c_str());
    cout<<"Verificando Basura"<<endl;
    vector<string>ipsNow = IPS;
    while((d = readdir(dir))!= NULL){
        if(d->d_type == DT_REG){
            actual= string(d->d_name);
            eliminando=actual;
            cout<<"Eliminando "<<actual<<endl;
            strcpy(pet.nombre, actual.c_str());
            pet.codigo = 3; 
            fallo=true;
            //Enviar a todas las IPS en el momento? 
            for(string ip : ipsNow){
                cout<<"Enviando a "<<ip<<endl;
                PaqueteDatagrama peticion((char*)&pet, sizeof(Peticion), (char*) ip.c_str(), puertoEscucha);
                for(int i=0; i<noFallos;i++){
                    s.envia(peticion);
                    r = s.recibeTimeout(res);    
                    if(r > 0){
                        fallo=false;
                        break;
                    }
                }
                if(fallo == true){
                    desconectar(ip);
                }
            }
            Archivos.erase(actual);
            completo= basura+actual;
            cout<<"Borrando archivo"<<endl;
            if( access(completo.c_str(), F_OK) != -1 ){
                if( remove(completo.c_str()) == -1 ){
                    cout << "Error al remover archivo \"" << pet.nombre << "\"" << endl;
                }
            }
        }
        eliminando="";
    }            
    closedir(dir);
}

void* manejoDirectorios(void* args){
    SocketDatagrama s(puertoArchivos);
    //Cargar archivos iniciales
    //vector<string> archivos= arranque();
    while(1){
        anunciarPropios(s);
        pedirFaltantes(s);
        //verificarCambios(archivos);
        eliminar();
        sleep(10);
    }
}