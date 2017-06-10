import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Hashtable;
import java.util.Queue;
import java.util.Vector;
import java.util.LinkedList;

class servicio extends Thread{
	//Estructuras Globales
	Hashtable<String, Vector> Archivos= new Hashtable<String, Vector>();
	Queue<String> Pendientes = new LinkedList<String>();
	Vector IPS = new Vector();

	/*Códigos de error */
	private static final int OK            =  0;
	private static final int E_BAD_OPCODE  =  -1;
	private static final int E_BAD_PARAM   =  -2;
	private static final int E_IO          =  -3;

	String directorio="/home/Carpeta/";
	String basura="/home/Basura/";
	String dirBroad="192.168.43.255";
	InetAddress direccionBroadcast = null;
	String eliminando="";

	int puertoServicios=7744;
	int puertoEliminar=7745;
	int puertoEscucha=7746;
	int puertoArchivos=7747;
	int noFallos=7;

	DatagramSocket socketServicio = null;

	int tipo;

	public servicio(int t){
		try{
			tipo=t;
			socketServicio=new DatagramSocket(puertoServicios);	
			direccionBroadcast = InetAddress.getByName(dirBroad);		
		}catch(IOException ioe){
			System.out.println("ERRROR EN CREAR EL SOCKET");
		}
	}

	public boolean existe(String a, Vector v){
		for(string b: v)
        	if(a.equals(b)) return true;
    	return false;
	}

	public String siguienteValido(Vector direcciones, int i, int largo){
		int inc=0;
		while(inc < largo){
			for(String ip: IPS){
				if(ip.equals(direcciones[(i+inc)%largo])) return ip;
			}
			inc++;
		}
		return "";
	}	

	public void desconectar(String ip){
		for(int i=0;i<IPS.size();i++){
	        if(ip==IPS[i]){
	            IPS.remove(i);
	            break;
	        }
	    }
	    return;
	}

	public void broadcast(){
	    ByteBuffer byteBuffer = ByteBuffer.allocate(8); 
	    byteBuffer.putInt(1);
	    byteBuffer.putInt(5);
	    byte [] b = byteBuffer.array();

	    while(1){
	        DatagramPacket p = new DatagramPacket(b, b.length, direccionBroadcast, puertoServicios);
	        socketServicio.send(p);
	        Thread.sleep(5000);
	        //IPS.clear();
	    }
	}

	public void detectarServicios(){
		byte buff = new byte[4];
		DatagramPacket p2;
		while(true){
			p2= new DatagramPacket(buff, buff.size());
			socketServicio.receive(p2);
			String a= p2.getAddress().getHostAddress();
			if(!existe(a, IPS))
	            IPS.add(string(p2.obtieneDireccion()));
		}
	}

	public void anunciarPropios(DatagramSocket socket){
		String nuevo;
		Peticion pet;
		byte [] b;
		pet.setCodigo(1);
		socket.setBroadcast(true);
		File dir = new File(directorio);
		String[] listado = dir.list();
		for (int i=0; i<listado.length; i++){
			if(Archivos.get(listado[i]) == null){
				Archivos.put(listado[i], new Vector());
			}
			pet.setNombre(listado[i]);
			b=pet.getByteRepr();
			DatagramPacket p = new DatagramPacket(b, b.length, direccionBroadcast, puertoEscucha);
			socket.send(p);
		}
	}

	public void pedirFaltantes(DatagramSocket socket){
		String actual;
		Peticion pet;
		Respuesta res;
		Vector direcciones;
		String dir;

		pet.setCodigo(2);
		int i=0, e, offset;
		boolean ban=true;

		socket.setSoTimeout(5000);
		int noIPS =  direcciones.size();

		while(Pendientes.peek() != null){
			actual = Pendientes.remove();
			ban= true;
			offset= 0;
			i=0;
			pet.setNombre(actual);
			direcciones= Archivos.get(actual);
			noIPS= direcciones.size();

			//Aqui se tiene que crear los archivos
		}
	}


	/*
	
	        Pendientes.pop();
	        ban=true;
	        offset=0;
	        i=0;
	        strcpy(pet.nombre, actual.c_str());
	        direcciones= Archivos[actual];
	        noIPS=direcciones.size();
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
	                //Esto deberia de mostrarse una excepcion o asi... no se me ha ocurrido nada
	                Pendientes.push(actual);
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
	*/

	public void manejoDirectorios(){
		DatagramSocket s(puertoArchivos);
		while(true){
			anunciarPropios(s);
			pedirFaltantes(s);
			//eliminar();
			Thread.sleep(1000);
		}
	}

	@Override
	public void run(){
		while(true){
			switch(tipo){
			case 1: 
				detectarServicios();
				break;
			case 2:
				broadcast();
				break;
			case 3:
				escuchar();
				break;
			case 4:
				manejoDirectorios();
				break;
			}
		}
	}
}