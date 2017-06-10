import java.io.*;
import java.net.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class Respuesta{
	byte [] data;
    int count;
    int result;
    public static int BUF_SIZE=5024;

	public Respuesta(){
		data= new byte[BUF_SIZE];
	}

	public byte[] getByteRepr() {
		ByteBuffer bb = ByteBuffer.allocate(BUF_SIZE+8);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		bb.put(data, 0, BUF_SIZE);
		bb.putInt(BUF_SIZE, count);
		bb.putInt(BUF_SIZE+4, result);
		return bb.array();
	}

	public void getClassFromBytes(byte[] buf) {
		ByteBuffer bb = ByteBuffer.wrap(buf);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		bb.position(0);
		bb.get(data);
		count = bb.getInt();
		result = bb.getInt();		
	}

	public void setData(byte [] n){
		data=n;
	}
	public void setCount(int o){
		count=o;
	}
	public void setResult(int c){
		result=c;
	}

	public byte [] getData(){
		return data;
	}
	public int getCount(){
		return count;
	}
	public int getResult(){
		return result;
	}

	public void print(){
		System.out.println(count +" "+ result+"\n");
	}
}