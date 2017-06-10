import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class Respuesta{
	String path;
	String pattern;
	int MAX_PATH= 256;
	public messageCS(String p1, String p2){
		path=p1;
		pattern=p2;
	}
	public byte[] getByteRepr() {
		ByteBuffer bb = ByteBuffer.allocate(2* MAX_PATH);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		for (int i = 0; i < this.path.length(); i++)
			bb.put((byte) this.path.charAt(i));
		for (int i = 0; i < this.pattern.length(); i++)
			bb.put(MAX_PATH+i,(byte) this.pattern.charAt(i));
		return bb.array();
	}
}