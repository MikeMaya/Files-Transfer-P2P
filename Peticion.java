import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class Peticion{
	int codigo;
	int offset;
    String nombre;
    
	public static int NAME_SIZE=1024;

	public Peticion(int i){
		codigo=i;
	}

	public byte[] getByteRepr() {
		ByteBuffer bb = ByteBuffer.allocate(NAME_SIZE+8);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		bb.putInt(codigo);
		bb.putInt(offset);
		for (int i = 0; i < this.nombre.length(); i++)
			bb.put((byte) this.nombre.charAt(i));
		return bb.array();
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

	public void setNombre(String n){
		nombre=n;
	}
	public void setOffset(int o){
		offset=o;
	}
	public void setCodigo(int c){
		codigo=c;
	}

	public String getNombre(){
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
