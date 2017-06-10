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
		DatagramSocket socketServicio=new DatagramSocket(7744);	
		servicio detectarServicios = new servicio(1);
		servicio broadcast= new servicio(2);
		servicio escuchar= new servicio(3);
		servicio manejoDirectorios= new servicio(4);
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