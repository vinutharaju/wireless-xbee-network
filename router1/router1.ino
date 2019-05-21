#include<SoftwareSerial.h>
const int trigPin = 3;
const int echoPin = 2;
float id = 10.01;
char dis_arr [10];
char id_arr[10];
char sendstring [10];
String receivestring;
long duration,distance;
SoftwareSerial Xbee(6, 7); // RX, TX

void setup()  {
  Serial.begin(9600);
  Xbee.begin(9600);
   pinMode(trigPin, OUTPUT);
   pinMode(echoPin, INPUT);
}
void loop()  {
   if (Xbee.available() >0) {
    char c = Xbee.read ();                   // gets one byte from serial buffer
    receivestring += c;                        // construct the recievestring_ch
    Serial.print("receivestring = ");
    Xbee.print(receivestring);
    Serial.println(receivestring);
    }

    else
    {
  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration*0.034/2;
  dtostrf(id,0,2,id_arr);
  strcat(sendstring,id_arr);
  float dis=distance;
  dtostrf(dis, 0, 2, dis_arr);
  strcat (sendstring, dis_arr);
  Xbee.write(sendstring);
  Serial.println(sendstring);
  delay (200);   
  memset(sendstring, '\0', 10);
  delay(3000);
}
}
