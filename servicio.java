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
			direccionBroadcast = InetAddress.getByName(dirBroad);		
		}catch(IOException ioe){
			System.out.println("ERRROR EN CREAR EL SOCKET");
		}
	}

	public void setSocket(DatagramSocket s){
		socketServicio=s;
	}

	public boolean existe(String a, Vector v){
		for(int i=0;i<v.size();i++)
        	if(a.equals(v.get(i))) return true;
    	return false;
	}

	public String siguienteValido(Vector direcciones, int i, int largo){
		int inc=0;
		while(inc < largo){
			for(int j=0;j<IPS.size();j++){
				if(IPS.get(j).equals(direcciones.get((i+inc)%largo))) return (String) IPS.get(j);
			}
			inc++;
		}
		return "";
	}	

	public void desconectar(String ip){
		for(int i=0;i<IPS.size();i++){
	        if(ip.equals(IPS.get(i))){
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

	    while(true){
	        DatagramPacket p = new DatagramPacket(b, b.length, direccionBroadcast, puertoServicios);
	        try{
	        	socketServicio.send(p);
		        Thread.sleep(5000);
	        }catch(IOException ioe){
	        	ioe.printStackTrace();
	        }catch(InterruptedException ie){
	        	ie.printStackTrace();
	        }
	        
	    }
	}

	public void detectarServicios(){
		byte [] buff = new byte[4];
		DatagramPacket p2;
		while(true){
			p2= new DatagramPacket(buff, buff.length);
			try{
				socketServicio.receive(p2);
			} catch(IOException ioe){
				ioe.printStackTrace();
			}
			String a= p2.getAddress().getHostAddress();
			if(!existe(a, IPS))
	            IPS.add(a);
		}
	}

	public void anunciarPropios(DatagramSocket socket){
		String nuevo;
		Peticion pet=new Peticion(1);
		byte [] b;
		try{
			socket.setBroadcast(true);
			File dir = new File(directorio);
			String[] listado = dir.listFiles();
			for (int i=0; i<listado.length; i++){
				if(Archivos.get(listado[i]) == null){
					Archivos.put(listado[i], new Vector());
				}
				pet.setNombre(listado[i]);
				b=pet.getByteRepr();
				DatagramPacket p = new DatagramPacket(b, b.length, direccionBroadcast, puertoEscucha);
				socket.send(p);
			}
		}catch(SocketException se){
			se.printStackTrace();
		}catch(IOException ioe){
			ioe.printStackTrace();
		}
	}

	public void pedirFaltantes(DatagramSocket socket){
		String actual;
		Peticion pet = new Peticion(2);
		Respuesta res;
		Vector direcciones;
		String dir;

		int i=0, e, offset;
		boolean ban=true;
		try {
			socket.setSoTimeout(5000);
		} catch(SocketException se){
			se.printStackTrace();
		}

		int noIPS = 0;

		while(Pendientes.peek() != null){
			actual = Pendientes.remove();
			ban= true;
			offset= 0;
			i=0;
			pet.setNombre(actual);
			direcciones= Archivos.get(actual);
			noIPS= direcciones.size();
			byte [] b;
			byte [] buff = new byte[4];
			//Aqui se tiene que crear los archivos
			FileOutputStream fop = null;
			File file;
			try{
				file = new File(directorio+actual);
				fop = new FileOutputStream(file);
				if (!file.exists()) {
					file.createNewFile();
				}
				while(ban){
					pet.setOffset(offset);
					dir= siguienteValido(direcciones, i, noIPS);
					if(dir.isEmpty()){
						Pendientes.add(actual);
						break;
					}
					b= pet.getByteRepr();
					DatagramPacket p(b,b.length,InetAddress.getByName(dir) ,puertoEscucha);
					DatagramPacket p2= new DatagramPacket(buff, buff.length);
					try {
						socket.send(p);
						socket.receive(p2);
						res = new Respuesta();
						res.getClassFromBytes(p2.getData());
						offset= res.getCount();

					} catch(){

					}
				}
			}
			
		}
	}


	/*
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

	public void eliminar(){
		//try{
			DatagramSocket s = new DatagramSocket(puertoEliminar);
        	Peticion pet = new Peticion(3);
        	byte buf [] = new byte[4];
			Vector <String> IPSnow =IPS;
            DatagramPacket paq = new DatagramPacket(buf, buf.length);
 			boolean fallo = true;
            String actual, completo;
            File dir = new File(basura);
            byte [] b;
            s.setSoTimeout(1000);
        //} catch(){

        //}
		String[] listado = dir.listFiles();
		for (int i=0; i<listado.length; i++){
            actual = listado[i];
            eliminando = actual;
            pet.setNombre(actual);
            fallo = true;
            for(int j = 0; j<IPSnow.size(); j++){
            	b = pet.getByteRepr();
                DatagramPacket petElim = 
                new DatagramPacket(b, b.length, InetAddress.getByName(IPSnow.get(j)), puertoEliminar);
                for(int j=0; j<noFallos; j++){
                    s.send(petElim); //envia(peticion);
                    //try{
                      s.receive(paq);
                        fallo=false;
                        break;
                    //} catch (){
                    //}
                }
                if(fallo){
                	desconectar(IPS-get(j));
                }
            }
            Archivos.remove(actual);
            completo= basura+actual;
            File borrar = new File(completo);
            if (borrar.delete())
                System.out.println("El fichero ha sido borrado satisfactoriamente");
            else
                System.out.println("El fichero no puede ser borrado");
            eliminando="";
        }
    }
           

	public void manejoDirectorios(){
		try{
			DatagramSocket s= new DatagramSocket(puertoArchivos);
			while(true){
				anunciarPropios(s);
				pedirFaltantes(s);
				//eliminar();
				Thread.sleep(1000);
			}
		}catch(SocketException se){
			se.printStackTrace();
		}catch(InterruptedException ie){
			ie.printStackTrace();
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
				//escuchar();
				break;
			case 4:
				manejoDirectorios();
				break;
			}
		}
	}
}