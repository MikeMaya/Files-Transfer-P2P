import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class Peticion{
	int offset;
	String name;
	int MAX_PATH= 256;

	public messageSC(){
	}

	public void getClassFromBytes(byte[] buf) {
		ByteBuffer bb = ByteBuffer.wrap(buf);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		offset = bb.getInt();
		StringBuilder sb = new StringBuilder();
		byte nameByte;
		while ( (nameByte = bb.get()) != '\0' )
			sb.append((char) nameByte);
		name=sb.toString();
	}

	public String getName(){
		return name;
	}

	public int getOffset(){
		return offset;
	}
	public void print(){
		System.out.println(name + " " + offset+"\n");
	}
}
