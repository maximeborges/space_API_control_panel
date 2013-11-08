
/*
 * =====================================================================================
 *
 *       Filename:  boite_status.ino
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/11/2013 13:13:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Sebastien Chassot (sinux), seba.ptl@sinux.net
 *        Company:  PostTenebrasLab the geneva's hackerspace   posttenebraslab.ch
 *
 * =====================================================================================
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SoftPWM.h>

/* PINOUT */
/* input */
int buttons = 6;
// int switchButtonPin = 2;      /* bouton du millieu  -  ( pinD2 'IRQ' )   */
// int RightLeverRightPin = 5;   /* levier de droite position droite - ( pinD5 'MOSI' ) */
// int RightLeverLeftPin = 6;    /* (pinD6 - 'MISO' )  */
// int LeftLeverRightPin = 7;    /* levier de gauche  position droite - ( pinD7 = ' SCK' ) */
// int LeftLeverLeftPin = 8;     /* ( pinD8 'CSn' )  */

/* output  */
int topLedRedPin = 14;    /*  MOSI  */
// int topLedBluePin = 1;   /*  MISO  */
// int topLedGreenPin = 2;  /*  SCK   */
// int lowLedPin = 3;       /*  CSn   */
// int amperemeterPin = 4;  /*  IRQ   */
// int voltmeterPin = 5;

int amperemeter = 0;   /* Ampermeter ooutput  */

void listenClient(int);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xC7, 0xDD, 0x54, 0xFF, 0xF4, 0xD1 };   /*  mac address randomly generated (no sticker on the board)  */
IPAddress ip(192,168,80,249);   /* fixed address - the device is named pannello.lan.posttenebraslab.ch */

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(115200);
//   while (!Serial) {
//    ; // wait for serial port to connect. Needed for Leonardo only
//  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  pinMode(buttons, INPUT);
//  pinMode(switchButtonPin, INPUT);
//  pinMode(RightLeverRightPin, INPUT);
//  pinMode(RightLeverLeftPin, INPUT);
//  pinMode(LeftLeverRightPin, INPUT);
//  pinMode(LeftLeverLeftPin, INPUT);

  SoftPWMBegin();
  
  SoftPWMSet(14, 0);

  SoftPWMSetFadeTime(14, 10000, 10000);
//  pinMode(topLedRedPin, OUTPUT);
//  analogWrite(topLedRedPin, 100);

//  pinMode(topLedBluePin, OUTPUT);
//  digitalWrite(topLedBluePin, LOW);
//  pinMode(topLedGreenPin, OUTPUT);
//  digitalWrite(topLedGreenPin, LOW);
//  pinMode(lowLedPin, OUTPUT);
//  digitalWrite(lowLedPin, LOW);
//  pinMode(amperemeterPin, OUTPUT);
//  digitalWrite(amperemeterPin, LOW);
//  pinMode(voltmeterPin, OUTPUT);
//  digitalWrite(voltmeterPin, LOW);

}


void loop() {

  int RightLeverActivatedDuration = millis() / 1000 ;
  
//  if(RightLeverRightPin == HIGH){
//    int increments = 10;
//    if( amperemeter <= 1023 - increments ){
//      amperemeter += increments;
//    }else{
//      amperemeter = 1023;
//    }
//  }
//  if(LeftLeverRightPin == HIGH){
//    int increments = 10;
//    if( amperemeter <= 1023 - increments ){
//      amperemeter += increments;
//    }else{
//      amperemeter = 1023;
//    }
//  }

  listenClient(RightLeverActivatedDuration);

  int i;
  for( i = 0; i <255; i++)
  {
    SoftPWMSet(14, i);
    SoftPWMSet(15, 255-i);
    delay(30);
  }
  
}

/* On envoie au lient ce qu'on veut */
void listenClient(int time){

// listen for incoming clients
EthernetClient client = server.available();
if (client) {
  Serial.println("new client");
  // an http request ends with a blank line
  boolean currentLineIsBlank = true;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);
      // if you've gotten to the end of the line (received a newline
      // character) and the line is blank, the http request has ended,
      // so you can send a reply
      if (c == '\n' && currentLineIsBlank) {
        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
	  client.println("Refresh: 5");  // refresh the page automatically every 5 sec
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        // output the value of each analog input pin
        for (int analogChannel = 2; analogChannel < 6; analogChannel++) {
          int sensorReading = analogRead(analogChannel);
          client.print("analog input ");
          client.print(analogChannel);
          client.print(" is ");
          client.print(sensorReading);
          client.println("<br />");    
        }
        client.print("La valeur de amperemeter est : ");   
        client.println(amperemeter);
        client.print("Le temps d'execusion est de: ");   
        client.println(time);

        
        client.println("</html>");
        break;
      }
      if (c == '\n') {
        // you're starting a new line
        currentLineIsBlank = true;
      } 
      else if (c != '\r') {
        // you've gotten a character on the current line
        currentLineIsBlank = false;
      }
    }
  }
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
  Serial.println("client disonnected");
  }
}
