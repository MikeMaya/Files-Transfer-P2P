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

		Hashtable<String, Vector> Archivos= new Hashtable<String, Vector>();
		Queue<String> Pendientes = new LinkedList<String>();
		Vector IPS = new Vector();

		DatagramSocket socketServicio=new DatagramSocket(7744);	
		servicio detectarServicios = new servicio(1);
		servicio broadcast= new servicio(2);
		servicio escuchar= new servicio(3);
		servicio manejoDirectorios= new servicio(4);

		detectarServicios.setEstructuras(Archivos, Pendientes, IPS);
		broadcast.setEstructuras(Archivos, Pendientes, IPS);
		escuchar.setEstructuras(Archivos, Pendientes, IPS);
		manejoDirectorios.setEstructuras(Archivos, Pendientes, IPS);

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