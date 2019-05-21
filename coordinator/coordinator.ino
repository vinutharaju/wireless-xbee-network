#include<SoftwareSerial.h>
#include <Arduino.h>
#include <Stream.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
//AWS
#include "sha256.h"
#include "Utils.h"
//WEBSockets
#include <Hash.h>
#include <WebSocketsClient.h>
//MQTT PUBSUBCLIENT LIB 
#include <PubSubClient.h>
//AWS MQTT Websocket
#include "Client.h"
#include "AWSWebSocketClient.h"
#include "CircularByteBuffer.h"
#include <stdlib.h>
extern "C" {
  #include "user_interface.h"
}
SoftwareSerial recv(4,5);//D2 1 RX TX

//AWS IOT config, change these:
char wifi_ssid[]       = "Airtel-E5573-F837";
char wifi_password[]   = "48ryfi2e";
char aws_endpoint[]    = "a3d3x4sh5k4yie-ats.iot.us-west-2.amazonaws.com";
char aws_key[]         = "AKIAISSXPVRO2DPAMNKA";
char aws_secret[]      = "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";
char aws_region[]      = "us-west-2";
const char* aws_topic  = "$aws/things/waterlevel/shadow/update";
int port = 443;

//MQTT config
const int maxMQTTpackageSize = 512;
const int maxMQTTMessageHandlers = 1;

ESP8266WiFiMulti WiFiMulti;
AWSWebSocketClient awsWSclient(1000);
PubSubClient client(awsWSclient);


//# of connections
long connection = 0;
String receivestring;
String idstring;
String disstring;
char idstring_arr [6]; 
char disstring_arr [6];
char c_id[15],c_dis[15];

//generate random mqtt clientID
char* generateClientID () {
  char* cID = new char[23]();
  for (int i=0; i<22; i+=1)
    cID[i]=(char)random(1, 256);
  return cID;
}
//count messages arrived
int arrivedcount = 0;
//callback to handle mqtt messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
//connects to websocket layer and mqtt layer
bool connect () {
    if (client.connected()) {    
        client.disconnect ();
    }  
    //delay is not necessary... it just help us to get a "trustful" heap space value
    delay (100);
    Serial.print (millis ());
    Serial.print (" - conn: ");
    Serial.print (++connection);
    Serial.print (" - (");
    Serial.print (ESP.getFreeHeap ());
    Serial.println (")");

    //creating random client id
    char* clientID = generateClientID ();
    
    client.setServer(aws_endpoint, port);
    if (client.connect(clientID)) {
      Serial.println("connected");     
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      return false;
    }
    
}

//subscribe to a mqtt topic
void subscribe () {
    client.setCallback(callback);
    client.subscribe(aws_topic);
   //subscript to a topic
    Serial.println("MQTT subscribed");
}

//send a message to a mqtt topic
void sendmessage () {
  delay(3000);
while(recv.available()>0){ 
       delay(10);
       if (recv.available() >0) {
            char c = recv.read ();                   // gets one byte from serial buffer
            receivestring += c;                        // construct the recievestring_ch
       }
    }
    
   if (receivestring.length() >0) {
       Serial.print ("receivestring = ");                // see what was received
       Serial.println (receivestring);                   // see what was received
     
// == expect a string like 10.0160.00 containing the id and distance ==  

       idstring = receivestring.substring (0, 5);      // get the first five characters
       disstring = receivestring.substring (5, 10);       // get the next five characters

       receivestring = ""; //  
       Serial.print ("idstring = ");                       
       Serial.println (idstring);                      // see what was received
       Serial.print ("disstring = ");                 
       Serial.println (disstring);                       // see what was received
       Serial.println ();                                // clear the receivestring_ch

       float id_fl = 0.00;                             // declare float for id
       float dis_fl = 0.00;                              // declare float for distance
     
   //  magic needed to convert string to float
       idstring.toCharArray(idstring_arr, sizeof(idstring_arr));
       id_fl = atof(idstring_arr);
       disstring.toCharArray(disstring_arr, sizeof(disstring_arr));
       dis_fl = atof(disstring_arr);

       dtostrf(id_fl,6,2,c_id);
       dtostrf(dis_fl,6,2,c_dis);
  
Serial.println();
//send message
   char buf[100];  
  strcpy(buf, "{\"state\":{\"reported\": {\"id\":");
  strcat(buf, c_id);
  strcat(buf,", \"dis\":");
  strcat(buf,c_dis);
  strcat(buf, "}}}");
  Serial.print(buf);
  int rc = client.publish(aws_topic, buf);                          
   }
}
void setup() {
    wifi_set_sleep_type(NONE_SLEEP_T);
    Serial.begin (9600);
    delay (2000);
    Serial.setDebugOutput(1);

    //fill with ssid and wifi password
    WiFiMulti.addAP(wifi_ssid, wifi_password);
    Serial.println ("connecting to wifi");
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        Serial.print (".");
    }
    Serial.println ("\nconnected");


    //fill AWS parameters    
    awsWSclient.setAWSRegion(aws_region);
    awsWSclient.setAWSDomain(aws_endpoint);
    awsWSclient.setAWSKeyID(aws_key);
    awsWSclient.setAWSSecretKey(aws_secret);
    awsWSclient.setUseSSL(true);   
}

void loop() {
  //keep the mqtt up and running
  if (awsWSclient.connected ()) {    
      subscribe (); 
      sendmessage ();  
       client.loop (); 
    }
    
   else {
    if (connect ()){
    //handle reconnection
    }
   }
}
