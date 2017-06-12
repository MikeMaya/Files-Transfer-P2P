import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Hashtable;
import java.util.Queue;
import java.util.Vector;
import java.util.LinkedList;

class cliente{


	public static void main(String args[]) throws Exception{
		if( args.length != 2 ){
			System.out.println("Uso: java cliente carpeta_nodo carpeta_basura");
			return;
		}

		String directorio = args[0];
		String basura = args[1];

		Hashtable<String, Vector> Archivos= new Hashtable<String, Vector>();
		Queue<String> Pendientes = new LinkedList<String>();
		Vector IPS = new Vector();

		DatagramSocket socketServicio=new DatagramSocket(7744);	
		servicio detectarServicios = new servicio(1);
		servicio broadcast= new servicio(2);
		servicio escuchar= new servicio(3);
		servicio manejoDirectorios= new servicio(4);

		detectarServicios.setEstructuras(Archivos, Pendientes, IPS, directorio, basura);
		broadcast.setEstructuras(Archivos, Pendientes, IPS, directorio, basura);
		escuchar.setEstructuras(Archivos, Pendientes, IPS, directorio, basura);
		manejoDirectorios.setEstructuras(Archivos, Pendientes, IPS, directorio, basura);

		//Seteamos el socket compartido
		detectarServicios.setSocket(socketServicio);
		broadcast.setSocket(socketServicio);
		//Iniciamos los serviocs
		detectarServicios.start();
		broadcast.start();
		escuchar.start();
		manejoDirectorios.start();
		
	}
}