package net.runphp.qqwry;

/*
 * 学习java时重新写了下lumaQQ作者马若劼的查询ip程序
 * 可以ip和地址双向查询,没有添加提高效率的代码,欢迎各位修改发布
 *
 * Copyright (C) 2004 luma <stubma@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

import java.io.*;
import java.util.*;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;

/**
 * 纯真ip查询主程序
 * QQWry.Dat保存在当前目录
 * @author runphp
 * @version 0.1, 04/23/08
 */
public class IpAddress {

    //数据库地址
    private String dataPath = "../qqwry.dat";
    //随机文件访问类
    private RandomAccessFile ipFile = null;
    //单一模式实例
    private static IpAddress instance = new IpAddress();
    //ip开始结束位置
    private long ipBegin=0L;
    private long ipEnd=0L;
    //ip总数
    private long ipSum=0L;
    //国家,地区
    private String country="";
    private String area="";

    // 一些固定常量，比如记录长度等等
    private static final int RECORD_LENGTH = 7;
    private static final byte AREA_FOLLOWED = 0x01;
    private static final byte NO_AREA = 0x02;

    /*
     * 私有构造函数
     */
    private IpAddress() {
        try {
            ipFile = new RandomAccessFile(new File(dataPath).getAbsolutePath(), "r");
        } catch (FileNotFoundException e) {
            System.out.println("IP地址信息文件没有找到，IP显示功能将无法使用");
        }
        if(ipFile != null) {
            try {
                ipBegin = byteArrayToLong(readBytes(0,4));
                ipEnd = byteArrayToLong(readBytes(4,4));
                if(ipBegin == -1 || ipEnd == -1) {
                    ipFile.close();
                    ipFile = null;
                }
            } catch (IOException e) {
                System.out.println("IP地址信息文件格式有错误，IP显示功能将无法使用");
            }
        }
        ipSum = (ipEnd-ipBegin)/RECORD_LENGTH+1;
    }

    /**
     * 在指定位置读取一定数目的字节
     * @param offset 位置
     * @param num 多少个字节
     * @return ret
     */
    private byte[] readBytes(long offset, int num) {
        byte[] ret = new byte[num];
        try {
            ipFile.seek(offset);

            for(int i=0; i != num; i++) {
                ret[i] = ipFile.readByte();
            }
            return ret;
        } catch (IOException e) {
            System.out.println("读取文件失败_readBytes");
            return ret;
        }
    }

    /**
     * 当前位置读取一定数目的字节
     * @param num 多少个字节
     * @return ret
     */
    private byte[] readBytes(int num) {
        byte[] ret = new byte[num];
        try {
            for(int i=0; i != num; i++) {
                ret[i] = ipFile.readByte();
            }
            return ret;
        } catch (IOException e) {
            System.out.println("读取文件失败_readBytes");
            return ret;
        }
    }

    /**
     * 对little-endian字节序进行了转换
     * byte[]转换为long
     * @param b
     * @return ret
     */
    private long byteArrayToLong(byte[] b) {
        long ret = 0;
        for(int i=0; i<b.length; i++) {
            ret |= ( b[i] << (0x8*i) & (0xFF * (long)(Math.pow(0x100,i))) );
        }
        return ret;
    }

    /**
     * 对little-endian字节序进行了转换
     * @param ip ip的字节数组形式
     * @return ip的字符串形式
     */
    private String byteArrayToStringIp(byte[] ip) {
        StringBuffer sb = new StringBuffer();
        for(int i=ip.length-1; i>=0; i--) {
            sb.append(ip[i] & 0xFF);
            sb.append(".");
        }
        sb.deleteCharAt(sb.length()-1);
        return sb.toString();
    }

    /**
     * 把ip字符串转换为long型
     * @param ip
     * @return long
     */
    private long StingIpToLong(String ip) {
        String[] arr = ip.split("\\.");
        return (Long.valueOf(arr[0])*0x1000000 +
                Long.valueOf(arr[1])*0x10000 +
                Long.valueOf(arr[2])*0x100 +
                Long.valueOf(arr[3]));
    }

    /**
     * 搜索ip,二分法
     * @param ip ip字符串0.0.0.0到255.255.255.255
     * @return long ip所在位置
     */
    public long seekIp(String ip) {
        long tmp = StingIpToLong(ip);
        long i=0;
        long j=ipSum;
        long m = 0;
        long lm=0L;
        while(i<j) {
            m = (i+j)/2;
            lm = m*RECORD_LENGTH + ipBegin;
            if( tmp == byteArrayToLong(readBytes(lm, 4))){
                return byteArrayToLong(readBytes(3));
            }else if(j==(i+1)) {
                return byteArrayToLong(readBytes(3));
            }else if( tmp > byteArrayToLong(readBytes(lm, 4))){
                i = m;
            }else/* if( tmp < byteArrayToLong(readBytes(lm, 4)))*/{
                j = m;
            }
        }
        System.out.println("没有找到ip");
        return -1L;
    }
    private String readArea(long offset) throws IOException {
        ipFile.seek(offset);
        byte b = ipFile.readByte();
        if(b == 0x01 || b == 0x02) {
            long areaOffset =byteArrayToLong(readBytes(offset+1,3));
            //   if(areaOffset == 0)
            //       return "未知";
            //   else
            return readString(areaOffset);
        } else
            return readString(offset);
    }
    /**
     * 通过ip位置获取国家地区,
     * 参照纯真ip数据库结构
     * @param offset
     * @return 国家+地区
     */
    private String seekCountryArea(long offset) {
        try {
            ipFile.seek(offset + 4);
            byte b = ipFile.readByte();
            if(b == AREA_FOLLOWED) {
                long countryOffset = byteArrayToLong(readBytes(3));
                ipFile.seek(countryOffset);
                b = ipFile.readByte();
                if(b == NO_AREA) {
                    country = readString(byteArrayToLong(readBytes(3)));
                    ipFile.seek(countryOffset + 4);
                } else
                    country = readString(countryOffset);
                area = readArea(ipFile.getFilePointer());
            } else if(b == NO_AREA) {
                country = readString(byteArrayToLong(readBytes(3)));
                area = readArea(offset + 8);
            } else {
                country = readString(ipFile.getFilePointer() - 1);
                area = readArea(ipFile.getFilePointer());
            }
            return country+" "+area;
        } catch (IOException e) {
            return null;
        }
    }

    /**
     * 从offset偏移处读取一个以0结束的字符串
     * @param offset
     * @return ret 读取的字符串，出错返回空字符串
     */
    private String readString(long offset){
        try {
            ipFile.seek(offset);
            byte[] b = new byte[128];
            int i;
            for(i=0; (b.length != i) && ((b[i]=ipFile.readByte()) != 0); i++);
            String ret = new String(b, 0 , i, "GBK");
            ret = ret.trim();
            return (ret.equals("") ||
                    ret.indexOf("CZ88.NET") != -1 )?"未知":ret;
        } catch (IOException e) {
            System.out.println("读取文件失败_readString");
        }
        return "";
    }

    /**
     * 包含字符串的ip记录
     * @param addr 地址
     * @return IpRecord ip记录
     */

    public ArrayList<IpRecord> stringToIp(String addr) {
        ArrayList<IpRecord> ret = new ArrayList<IpRecord>();
        try{
            FileChannel fc = ipFile.getChannel();
            MappedByteBuffer mbb = fc.map(FileChannel.MapMode.READ_ONLY, 0, ipFile.length());
            mbb.order(ByteOrder.LITTLE_ENDIAN);
            //上面3代码未使用,内存映射文件功能未写

            for(long i = ipBegin+4; i != ipEnd+4; i += RECORD_LENGTH) {
                String sca = seekCountryArea(byteArrayToLong(readBytes(i, 3)));
                if(sca.indexOf(addr) != -1) {
                    IpRecord rec = new IpRecord();
                    rec.address = sca;
                    rec.beginIp= byteArrayToStringIp(readBytes(i-4,4));
                    rec.endIp= byteArrayToStringIp(readBytes(i+3,4));
                    ret.add(rec);
                }
            }
        } catch (IOException e) {
            System.out.println(e.getMessage());
        }
        return ret;
    }

    /**
     * 封装ip记录,包括开始ip,结束ip和地址
     */
    private class IpRecord {
        public String beginIp;
        public String endIp;
        public String address;

        public IpRecord() {
            beginIp = endIp = address = "";
        }

        public String toString() {
            return beginIp + " - " + endIp + " " + address;
        }
    }

    /**
     * @return 单一实例
     */
    public static IpAddress getInstance() {
        return instance;
    }

    /**
     * @param ip
     * @return ret
     */
    public String IpStringToAddress(String ip) {
        //这里要添加ip格式判断
        //public boolean isIP(Strin ip)
        long ipOffset = seekIp(ip);
        String ret = seekCountryArea(ipOffset);
        return ret;
    }

    /**
     * @return IpSum
     */
    public long getIpSum() {
        return ipSum;
    }

    public static void main(String[] args) {
        IpAddress ipAddr = IpAddress.getInstance();
        //ip总数
        long l = ipAddr.getIpSum();
        System.out.println(l);
        //纯真ip数据更新时间
        String str = ipAddr.IpStringToAddress("255.255.255.0");
        System.out.println(str);
        //测试
        str = ipAddr.IpStringToAddress("222.88.59.214");
        System.out.println(str);
        str = ipAddr.IpStringToAddress("222.248.70.78");
        System.out.println(str);
        str = ipAddr.IpStringToAddress("188.1.255.255");
        System.out.println(str);
        str = ipAddr.IpStringToAddress("220.168.59.166");
        System.out.println(str);
        str = ipAddr.IpStringToAddress("188.67.234.23");
        System.out.println(str);

   /*   java.net.InetAddress addr = null;
       try{
           addr = java.net.InetAddress.getLocalHost();
       }catch(java.net.UnknownHostException e){
           e.printStackTrace();
       }
       String ip=addr.getHostAddress().toString();//获得本机IP
       System.out.print(ip);
       String address=addr.getHostName().toString();//获得本机名称
       System.out.print(address);
       str = ipAddr.IpStringToAddress(ip);
       System.out.println(str);*/


      /*  ArrayList<IpRecord> al = ipAddr.stringToIp("网吧");
        Iterator it = al.iterator();

        File f = new File("ipdata.txt");
        try{
            if(!f.exists()) {
                f.createNewFile();
            }
            BufferedWriter out = new BufferedWriter(
                    new OutputStreamWriter(
                            new FileOutputStream(f, true)
                    )
            );
            int i=0;
            while(it.hasNext()) {
                out.write(it.next().toString());
                out.newLine();
                i++;
            }
            out.write(new Date().toString());
            out.write("总共搜索到 "+i);
            out.close();
        }catch(IOException e){
            e.printStackTrace();
        }*/

    }
}