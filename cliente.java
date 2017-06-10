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
	//Estructuras Globales
	Hashtable<String, Integer> Archivos= new Hashtable<String, Integer>();
	Queue<String> Pendientes = new LinkedList<String>();
	Vector IPS = new Vector();

	/*Códigos de error */
	private static final int OK            =  0;
	private static final int E_BAD_OPCODE  =  -1;
	private static final int E_BAD_PARAM   =  -2;
	private static final int E_IO          =  -3;

	String directorio="/home/Carpeta/";
	String basura="/home/Basura/";
	String direccionBroadcast="192.168.43.255";
	String eliminando="";

	int puertoServicios=7744;
	int puertoEliminar=7745;
	int puertoEscucha=7746;
	int puertoArchivos=7747;
	int noFallos=7;

	DatagramSocket socketServicio = null;

	InetAddress IPAddress = null;
	int puerto = 9876;
	int MAX_PATH= 256;

	public cliente(){
		try{
			socketServicio=new DatagramSocket(puertoServicios);			
		}catch(IOException ioe){
			System.out.println("ERRROR EN CREAR EL SOCKET");
		}
	}

	public static void main(String args[]) throws Exception{
		
	}
}
