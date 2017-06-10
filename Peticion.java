import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class Peticion{
	int codigo;
    String nombre;
    int offset;
	int NAME_SIZE=1024;

	public Peticion(){
	}

	public void getClassFromBytes(byte[] buf) {
		ByteBuffer bb = ByteBuffer.wrap(buf);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		codigo = bb.getInt();
		offset = bb.getInt();
		StringBuilder sb = new StringBuilder();
		byte nameByte;
		while ( (nameByte = bb.get()) != '\0' )
			sb.append((char) nameByte);
		nombre=sb.toString();
	}

	public String getName(){
		return nombre;
	}
	public int getOffset(){
		return offset;
	}
	public int getCodigo(){
		return codigo;
	}

	public void print(){
		System.out.println(codigo +" "+ nombre + " " + offset+"\n");
	}
}
