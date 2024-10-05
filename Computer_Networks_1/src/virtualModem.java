import ithakimodem.*;
import java.util.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.FileWriter;
import java.time.LocalDateTime;  
import java.text.DateFormat;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.lang.Math;

import java.util.concurrent.TimeUnit;



public class virtualModem {
    
    Modem modem = new Modem();

    public virtualModem(int speed, int timeout){
        modem.setSpeed(speed);
        modem.setTimeout(timeout);

        modem.open("ithaki");
        int k;

        for (;;){
            try{
                k=modem.read();
                if (k==-1) break;
                System.out.print((char)k);
            } catch (Exception x){
                break;
            }
        }
    }

    //Returns response time of a single echo packet
    public long echo(String echo_code){

        long before = System.currentTimeMillis();
        modem.write(echo_code.getBytes());
        int k;

        String end_suffix= "PSTOP"; //The end suffix of the echoPacket
        
        int i=0;

        for (;;){
            try{
                k=modem.read();
                //Timeout
                if (k==-1){
                    System.out.println("TIMEOUT!");
                    break;
                }
                //Check for end suffix
                if (k==end_suffix.charAt(i)){
                    i++;
                }else i=0;
                //If the entire end suffix has been detected
                if (i==end_suffix.length()){ 
                    long after = System.currentTimeMillis();
                    return(after - before);
                }
            } catch (Exception x){
                return -1;
            }
        }
        return -1;
    }

    //Saves incoming jpeg image
    public void getImage(String image_code){
        modem.write(image_code.getBytes());
        int k;
        try{
            FileOutputStream out = new FileOutputStream(new File("IthakiImage"+image_code.replace("\n", "")+".jpeg"));
            //End of image delimiter OxFF 0xD9
            int[] endOfImage=new int[]{Integer.parseInt("FF",16),Integer.parseInt("D9",16)};
            int i=0;

            for (;;){
                try{
                    k=modem.read();

                    //Timeout
                    if (k==-1){break;}
                    out.write(k);

                    //Check for end suffix
                    if (k==endOfImage[i]){
                        i++;
                    }else i=0;
                    //If the entire end suffix has been detected
                    if (i==endOfImage.length){break;}
                    

                } catch (Exception x){
                    break;
                }
            }
            out.close();
        }catch (IOException e) {
            e.printStackTrace(); 
        }
       
    }

    //Closes connection
    public void close_modem(){
        modem.close();
    }

    //Converts coordinates from DDMM.MMMMM format to DDMMSS format
    public String DDMtoDMS(String coordinate){
        String degrees=(coordinate.split("\\.")[0]).substring(coordinate.split("\\.")[0].length() - 4,coordinate.split("\\.")[0].length() - 2);
        String minutes=(coordinate.split("\\.")[0]).substring(coordinate.split("\\.")[0].length() - 2);
        String seconds= String.valueOf((Float.parseFloat("0."+coordinate.split("\\.")[1])*60)).split("\\.")[0];
        return degrees+minutes+seconds;
    }

    //Parses incoming GPS packets, selects coordinates that are at least 4 seconds appart 
    //and requests an image from the server using the getImage method above
    public void getGps(String gps_code, String rparam){

        String req_code=gps_code+rparam+"\r";
        modem.write(req_code.getBytes());
        int k;

        String GPS_suffix= "GPGGA"; //The end suffix of the echoPacket
        
        int i=0;
        Date previousGPS=null;
        List<String> latitudes = new ArrayList<String> (); 
        List<String> longitudes = new ArrayList<String> (); 
        
        for (;;){
            try{

                k=modem.read();
                //Timeout
                if (k==-1){
                    break;
                }
                //Check for end suffix
                if (k==GPS_suffix.charAt(i)){
                   i++;
                }else i=0;
                //If the entire end suffix has been detected
                if (i==GPS_suffix.length()){ 
                    String time = "";
                    String longitude ="";
                    String latitude ="";

                    //Parsing the GPS information
                    int p=14;
                    while(p>0){
                        k=modem.read();
                        if(k==','){p--;}
                        else{
                            if(p==13) {time+=(char)k;}
                            if(p==12) {latitude+=(char)k;}
                            if(p==10) {longitude+=(char)k;}
                        }
                    }

                    DateFormat timeFormat = new SimpleDateFormat("HHmmss");
                    Date gpsTime = timeFormat.parse(time.split("\\.")[0]);
                    String formatedLongitude = DDMtoDMS(longitude);
                    String formatedLatitude = DDMtoDMS(latitude);
                    if (previousGPS==null)
                    {
                        previousGPS=gpsTime;
                        latitudes.add(formatedLatitude);
                        longitudes.add(formatedLongitude);
                    }else if (gpsTime.getTime()-previousGPS.getTime()>4000)
                    {
                        previousGPS=gpsTime;
                        latitudes.add(formatedLatitude);
                        longitudes.add(formatedLongitude);
                    }
                    i=0;
                }
            } catch (Exception x){
                break;
            }
        }

        //Image request
        req_code=gps_code;
        ListIterator<String> it = longitudes.listIterator();
        while (it.hasNext()) {
            int index = it.nextIndex();
            String longitude = it.next();
            String latitude = latitudes.get(index);
            req_code+="T="+longitude+latitude;
        }
        req_code+="\r";
        getImage(req_code);
    }

    //Returns the time it took for a packet to arrive error free using AQR
    public String AQRpackets(String ack_code, String nack_code){
        long before = System.currentTimeMillis();
        boolean received = false;
        modem.write(ack_code.getBytes());

        //Total times packet was sent until received correctly
        int l=0;
        while(!received)
        {
            l++;
            int k;
            String end_suffix= "PSTOP"; //The end suffix of the packet
            
            int i=0;

            for (;;){
                try{
                    k=modem.read();
                    //Timeout
                    if (k==-1){break;}

                    //Check for end suffix
                    if (k==end_suffix.charAt(i)){i++;}else i=0;
                    
                    //If the entire end suffix has been detected
                    if (i==end_suffix.length()){ 
                        break;
                    }

                    //Information of packet
                    if((char)k=='<'){
                        k=modem.read();
                        int checkFCS=k;
                        k=modem.read();
                        while((char)k!='>'){
                            //XOR between each character in brackets
                            checkFCS =  checkFCS ^ k;
                            k=modem.read();
                        }
                        k=modem.read();

                        //Save incoming FCS as integer
                        int FCS = 0;
                        for (int j=0; j<3; j++) {
                            k=modem.read();
                            FCS += Character.getNumericValue((char)k)*Math.pow(10, 2-j);
                        }

                        //Check for errors
                        if(FCS==checkFCS){received=true;}
                    }
                } catch (Exception x){
                    return "E";
                }
            }

            if(received){
                return(System.currentTimeMillis() - before+","+l);
            }else{
                //Return NACK
                modem.write(nack_code.getBytes());
            }
        }
        return "E";
    }
    public static void main(String[] args) throws Exception {
        virtualModem my_modem = new virtualModem(80000,1000);

        //-----ECHO PACKET REQUESTS--------

        System.out.println("Echo requests start time: " + java.time.LocalTime.now());
        long start = System.currentTimeMillis();
        long timePassed=0;
        try{
            FileWriter myWriter = new FileWriter(new File("responseTimes.txt"));
            PrintWriter myPrintWriter = new PrintWriter(myWriter);
            while(timePassed<300000)
            {  
                timePassed=(System.currentTimeMillis()-start);
                myPrintWriter.println(my_modem.echo("E5043\r"));
            }
            myWriter.close();
            myPrintWriter.close();
        }catch (IOException e) {
            e.printStackTrace(); 
        }

        //-----IMAGE REQEUSTS-------- 

        // System.out.println("Image request start time (error free): " + java.time.LocalTime.now());       
        // my_modem.getImage("M1196\r");
        // System.out.println("Image request start time (with errors): " + java.time.LocalTime.now());
        // my_modem.getImage("G7929\r");

        //-----GPS REQEUSTS-------- 

        // System.out.println("GPS request start time: " + java.time.LocalTime.now());
        // my_modem.getGps("P3822","R=1000030");


        //-----ARQ PACKET REQUESTS--------

        // System.out.println("ARQ packet requests start time: " + java.time.LocalTime.now());
        // long AQRstart = System.currentTimeMillis();
        // long AQRtimePassed=0;
        // try{
        //     FileWriter myWriter = new FileWriter(new File("ARQresponseTimes.txt"));
        //     PrintWriter myPrintWriter = new PrintWriter(myWriter);
        //     while(AQRtimePassed<300000)
        //     {  
        //         AQRtimePassed=(System.currentTimeMillis()-AQRstart);
        //         myPrintWriter.println(my_modem.AQRpackets("Q5988\r", "R4305\r"));
        //     }
        //     myWriter.close();
        //     myPrintWriter.close();
        // }catch (IOException e) {
        //     e.printStackTrace(); 
        // }
        // my_modem.close_modem();
    }
}
