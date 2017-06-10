import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;
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

	String directorio="C:/Carpeta/";
	String basura="C:/Basura/";
	String dirBroad="192.168.1.255";
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
	        	//System.out.println("Enviando Direccion");
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
			//System.out.println("Recibio direccion: "+a);
			if(!existe(a, IPS))
	            IPS.add(a);
		}
	}

	public void escuchar(){
        try(DatagramSocket s = new DatagramSocket(puertoEscucha)){
            Peticion pet = new Peticion(1);
            Respuesta res = new Respuesta();

            byte[] bufferPet = new byte[Peticion.NAME_SIZE + 2*(Integer.SIZE/Byte.SIZE)];
            byte[] bufferRes = new byte[Respuesta.BUF_SIZE + 2*(Integer.SIZE/Byte.SIZE)];
            byte[] bufferCon = new byte[1];
            
            while(true){
                DatagramPacket paqpet = new DatagramPacket(bufferPet, bufferPet.length);
                DatagramPacket paqres = new DatagramPacket(bufferRes, bufferRes.length);
                DatagramPacket paqcon = new DatagramPacket(bufferCon, bufferCon.length);
                
                s.receive(paqpet);
                pet.getClassFromBytes(paqpet.getData());
                String ipRemota = paqpet.getAddress().getHostAddress();
                String filename = directorio + pet.nombre;
                System.out.println("Paquete " + ipRemota + " - " + pet.codigo + " - " + pet.nombre);

                switch(pet.codigo){
                    case 1: // Anuncio archivo
                        System.out.println("  Trabajando en: " + eliminando);
                        if( pet.nombre.equals(eliminando) )
                            break;
                        
                        if( !Archivos.containsKey(pet.nombre) ){
                            System.out.println("  Archivo a pendientes \"" + pet.nombre + "\"");
                            Pendientes.add(pet.nombre);
                            Archivos.put(pet.nombre, new Vector());
                        }
                        
                        Vector<String> v = Archivos.get(pet.nombre);
                        if( !v.contains(ipRemota) ){
                            System.out.println("  Primera vez de la ip " + ipRemota);
                            v.add(ipRemota);
                            Archivos.replace(pet.nombre, v);
                        }
                        break;
                        
                    case 2: // Peticion de un archivo
                        System.out.println("  Buscando para mandar el archivo " + filename);
                        
                        try(RandomAccessFile raf = new RandomAccessFile(filename, "rb")){
                            raf.seek(pet.offset);
                            res.setCount(raf.read(res.data, 0, Respuesta.BUF_SIZE));
                            res.setResult(OK);
                        } catch(FileNotFoundException e){
                            res.setResult(E_IO);
                        }
                        
                        paqres.setData(res.getByteRepr());
                        paqres.setAddress(paqpet.getAddress());
                        paqres.setPort(paqpet.getPort());
                        
                        s.send(paqres);
                        break;
                        
                    case 3: // Peticion de borrar un archivo
                        System.out.println("  Eliminando el archivo " + filename);
                        
                        Path path = Paths.get(filename);
                        Files.deleteIfExists(path);
                        
                        Archivos.remove(pet.nombre);
                        bufferCon[0] = 1;
                        paqcon.setData(bufferCon);
                        paqcon.setAddress(paqpet.getAddress());
                        paqcon.setPort(paqpet.getPort());
                        
                        s.send(paqcon);
                        break;
                        
                    default:
                        System.out.println("  Codigo no reconocido: " + pet.codigo);
                        System.out.print("    Direccion: " + paqpet.getAddress().getHostAddress());
                        System.out.println(" : " + paqpet.getPort());
                }
            }
        } catch(Exception e){
            System.out.println("**EN FUNCION ESCUCHAR:");
            e.printStackTrace();
        }
    }

	public void anunciarPropios(DatagramSocket socket){
		String nuevo;
		Peticion pet=new Peticion(1);
		byte [] b;
		try{
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
				System.out.println("Anunciando "+pet.getNombre());
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
			System.out.println("Pidiendo "+actual);
			try{
				file = new File(directorio+actual);
				fop = new FileOutputStream(file);
				if (!file.exists()) {
					file.createNewFile();
				}
				while(ban){
					pet.setOffset(offset);
					dir= siguienteValido(direcciones, i, noIPS);
					System.out.println("Paquete "+offset+" - "+dir);
					if(dir.isEmpty()){
						System.out.println("Direccion no encontrada para "+actual);
						Pendientes.add(actual);
						break;
					}
					b= pet.getByteRepr();
					try {
						DatagramPacket p =new DatagramPacket(b,b.length,InetAddress.getByName(dir) ,puertoEscucha);
						DatagramPacket p2= new DatagramPacket(buff, buff.length);
						socket.send(p);
						socket.receive(p2);
						res = new Respuesta();
						res.getClassFromBytes(p2.getData());
						offset= res.getCount();
						fop.write(res.getData());
						if(res.getCount() < res.BUF_SIZE){
							fop.close();
							ban=false;
						}
					} catch (SocketTimeoutException ste){
       					ste.printStackTrace();	
       				} 
					catch(UnknownHostException uhe){
						uhe.printStackTrace();
					} catch(IOException ioe){
						ioe.printStackTrace();
					}
					i= (i+1)%noIPS;
				}
			} catch(FileNotFoundException fnofe){
				fnofe.printStackTrace();
			} catch(IOException ioe){
				ioe.printStackTrace();
			}
			
		}
	}

	public void eliminar(){
		try{
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
			String[] listado = dir.list();
			for (int i=0; i<listado.length; i++){
	            actual = listado[i];
	            eliminando = actual;
	            pet.setNombre(actual);
	            fallo = true;
	            for(int j = 0; j<IPSnow.size(); j++){
	            	b = pet.getByteRepr();
	                DatagramPacket petElim = 
	                new DatagramPacket(b, b.length, InetAddress.getByName(IPSnow.get(j)), puertoEliminar);
	                for(int k=0; k<noFallos; k++){                 
	                    try{
	                    	s.send(petElim); //envia(peticion);
	                      	s.receive(paq);
	                        fallo=false;
	                        break;
	                    } catch(SocketTimeoutException ste) {
	                    	ste.printStackTrace();
	                    } catch (IOException ioe){
	                    	ioe.printStackTrace();
	                    }
	                }
	                if(fallo){
	                	desconectar((String) IPS.get(j));
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
        } catch(SocketException se){
        	se.printStackTrace();
        } catch(UnknownHostException uhe){
        	uhe.printStackTrace();
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
				System.out.println("Iniciando detectarServicios");
				detectarServicios();
				break;
			case 2:
				System.out.println("Iniciando broadcast");
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