import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class Respuesta{
	String data; 
    int count;
    int result;
    int BUF_SIZE=5024;

	public Respuesta(){
	}

	public byte[] getByteRepr() {
		ByteBuffer bb = ByteBuffer.allocate(BUF_SIZE+8);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		for (int i = 0; i < this.data.length(); i++)
			bb.put((byte) this.data.charAt(i));
		bb.putInt(BUF_SIZE,count);
		bb.putInt(BUF_SIZE+4,result);
		return bb.array();
	}
}