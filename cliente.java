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
	int puertoServicios=7744;

	public static void main(String args[]) throws Exception{
		DatagramSocket socketServicio=new DatagramSocket(puertoServicios);	
		servicio detectarServicios(1);
		servicio broadcast(2);
		servicio escuchar(3);
		servicio manejoDirectorios(4);
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