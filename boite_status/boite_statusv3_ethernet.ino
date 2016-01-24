/*
   =====================================================================================

         Filename:  boite_status.ino

      Description:

          Version:  3.0 (2016-01-24)
          Created:  02/05/2014
         Revision:  Ethernet shield version
         Compiler:  gcc

           Author:  Sebastien Chassot (sinux), seba.ptl@sinux.net
         Modified:  Alexandre Rosenberg
          Company:  PostTenebrasLab the geneva's hackerspace   posttenebraslab.ch

   =====================================================================================
     Alexandre Rosenberg - Added basic serial control (speed: 115200)
     "get 1", "get 2" for galva 1 and 2 value
     "set 1 x" to set galva 1 to value x, "set 2 x" for galva 2

     Alexandre Rosenberg - tweaked value and scale to non linear
     to match with the printed design.

*/

#include <SPI.h>
#include <Bounce.h>

#include <EthernetServer.h>
#include <Dhcp.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Ethernet.h>

/* Ethernet Shield*/
// MAC address of the Ethernet shield
byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x2C, 0x86 };

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

/* Founctions  */
void update_buttons();       /* debounce lib need an update every loop */
boolean light();             /* Is it some light in space true/false */

// API 

/* PINOUT */
/* Output pins are all PWM pins */
#define GALV1 5              /*  Galva 1 (gauche)  */
#define GALV2 6              /*  Galva 2 (droite)  */
#define LED1 9               /*  Led du haut Red */
#define LEDR A5              /*  Led du bas Red  */
#define LEDB A1              /*  Led du bas Blue  */
#define LEDG A3              /*  Led du bas Green  */

/* Input pins are digital pins */
#define OnOff 2              /* Interupteur du millieu On/Off */
#define LEVRR 7              /* Levier Right position Right */
#define LEVRL A4              /* Levier Right position Left */
#define LEVLR A2             /* Levier Left position Right */
#define LEVLL 8              /* Levier Left position Left */

/* Analog Input pins for photoresistor */
#define LIGHTPIN A0             /* Photoresistor

/* Const */
#define STEP 1               /* step de l'acceleration aiguille step+step*temps)  */
#define MINUTE 60000         /* Vitesse de descente de l'aiguille (60 secondes dans une minute)  */
#define GALV1RANGE 960       /* Le range du galva 1 960 == 16*60 min - 16 heures */
#define GALV2RANGE 160       /* Le range du galva 2 160. max 16 ppl ) */
#define BLINKPERIOD 3000     /* Frequence de cligotement du temoin rouge (local fermé) */
#define BLINKOn 500          /* durée de clignotement du temoin */
#define LASTHOUR 60          /* en se raprochant de 0 la LED va progressivement changer de couleur à partir de X minutes */
#define LIGHTTHRESOLD 100    /* Seuil de detection de presence (luminosite) */

#define LED1On digitalWrite(LED1, LOW)
#define LED1Off digitalWrite(LED1, HIGH)
#define LEDROn analogWrite(LEDR, 0)
#define LEDROff analogWrite(LEDR, 255)
#define LEDBOn analogWrite(LEDB, 0)
#define LEDBOff analogWrite(LEDB, 255)
#define LEDGOn analogWrite(LEDG, 0)
#define LEDGOff analogWrite(LEDG, 255)

/* les levier et switch utilisent la librairie Bounce.h  */
Bounce cancel = Bounce(OnOff, 20);        /* debounce 20 ms  */
Bounce levRR = Bounce(LEVRR, 20);
Bounce levRL = Bounce(LEVRL, 20);
Bounce levLR = Bounce(LEVLR, 20);
Bounce levLL = Bounce(LEVLL, 20);

int stateGalv1 = 0;
int stateGalv2 = 0;

long opentime = 0;
long old_opentime = 0;
long ppl_count = 0;

int minute = 0;
int ppl = 0;
int hour = 0;

unsigned long reftime = 0;
unsigned long ledtime = 0;          /* timer pour acceleration aiguille galva  */

unsigned long previousMillis = 0;   /* timer for updating the status           */
long t_interval = 60000;            /* Update interval in milisec              */

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean lightInSpace;

//used for serial command
int galva_no = 0;
int new_value = 0;

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
}

void setup() {

  // Open serial communications and wait for port to open:
  // initialize serial:
  Serial.begin(115200);
  Serial.println("Started Serial");

  // [Eth Shield] Start the Network connection
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Network DHCP");
    // no point in carrying on, so do nothing forevermore:
    while (true);
  }

  // [Eth Shield] Give a second to initialize:
  delay(1000);
  Serial.println("Configured IP using DHCP");
  printIPAddress();

  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  pinMode(OnOff, INPUT);
  pinMode(LEVRR, INPUT);
  pinMode(LEVRL, INPUT);
  pinMode(LEVLR, INPUT);
  pinMode(LEVLL, INPUT);

  pinMode(LED1, OUTPUT);      /* LED sous la coupole rouge (elle est blanche) */
  LED1Off;
  pinMode(LEDR, OUTPUT);      /* LED rouge sous la coupole blanche */
  //Serial.print("LED Rouge : ");
  //Serial.println(digitalRead(LEDR));
  LEDROff;
  pinMode(LEDB, OUTPUT);      /* LED bleue sous la coupole blanche */
  //Serial.print("LED Bleu : ");
  //Serial.println(digitalRead(LEDB));
  LEDBOff;
  pinMode(LEDG, OUTPUT);      /* LED verte sous la coupole blanche */
  //Serial.print("LED Verte : ");
  //Serial.println(digitalRead(LEDG));
  LEDGOff;
}


void loop() {

  /*
  if (light()) {
    //Serial.println("Il y a de la lumiere au PTL !!! ");
  }
  */

  update_buttons();

  /********************
     levier 1
   *********************/
  if ( !levRR.read() )
  {
    opentime = constrain( opentime + STEP + STEP * (long)levRR.duration() / 500, 0 , GALV1RANGE ); /* acceleration, valeur de 0 à GALV1RANGE */
    reftime = millis();
    delay(35);
  }
  else if ( !levRL.read() )
  {
    opentime = constrain( opentime - STEP -  STEP * (long)levRL.duration() / 500, 0 , GALV1RANGE ); /* acceleration, valeur de 0 à GALV1RANGE */
    delay(35);
  }

  /********************
     levier 2
   *********************/
  if ( !levLR.read() )
  {
    ppl_count = constrain( ppl_count + STEP + STEP * (long)levLR.duration() / 500, 0 , GALV2RANGE ); /* acceleration, valeur de 0 à GALV2RANGE */
    delay(35);
  }
  else if ( !levLL.read() )
  {
    ppl_count = constrain( ppl_count - STEP -  STEP * (long)levLL.duration() / 500, 0 , GALV2RANGE ); /* acceleration, valeur de 0 à GALV2RANGE */
    delay(35);
  }

  /********************
     Interrupteur
   *********************/
  if ( cancel.read() && ( cancel.duration() > 2000 ) )
  {
    opentime = 0;
    ppl_count = 0;
  }

  // If space is open, there should be at least one person !

  if ( ( opentime > 0 ) && ( ppl_count < 10 ) )
  {
    ppl_count = 10 ;
  }

  if ( opentime == 0 )
  {
    ppl_count = 0 ;
  }

  // AR Take into account non-linear scale. First 2 steps are (ppl / hour) 2x larger
  // Glav1 - hour counter
  if ( opentime <= 120 )
  {
    stateGalv1 = map(opentime, 0, 120, 0, 51);        /* valeur de 0 à 120 min double (255 // 18 *2*2) = 56. (tweeked to match)  */
  }

  if ( opentime > 120)
  {
    stateGalv1 = map(opentime - 120, 0, GALV1RANGE, 51, 258);
  }

  // Glav1 - ppl counter
  if ( ppl_count <= 20 )
  {
    stateGalv2 = map(ppl_count, 0, 20, 0, 51);
  }

  if ( ppl_count > 20)
  {
    stateGalv2 = map(ppl_count - 20, 0, GALV2RANGE, 51, 280);
  }

  /*  Une LED rouge clignotte lentement quand le local est fermé */
  if ( !opentime )
  {
    if ( (millis() - ledtime) > ( BLINKPERIOD + BLINKOn ) )
    {
      LED1Off;
      ledtime = millis();       /*  remise à zero du compteur  */

    }
    else if ( (millis() - ledtime) > BLINKPERIOD ) {
      LED1On;
    }
  }
  else {
    LED1Off;
  }

  /****************************************************
     Temps
   ***************************************************/

  /*  toute les minutes on decremente le temps */
  if ( (millis() - reftime) >= MINUTE )
  {
    opentime--;

    if (opentime < 0)
      opentime = 0;
    reftime = millis();
    // Serial.println("timer decrement");
  }

  if ( opentime != old_opentime )
  {
    old_opentime = opentime;
  }

  if ( opentime == 0 )              /*  la dernière heure on passe du vert au rouge  */
  {
    LEDROff;
    LEDBOff;
    LEDGOff;
  }
  else if ( opentime > 180 )      /* quand plus d'une heure et demi c'est vert */
  {
    LEDROff;
    LEDBOff;
    LEDGOn;
  }
  else {                            /*  Entre LASTHOUR et 0 on passe de Green à Red */
    analogWrite(LEDR, map(opentime, 0, LASTHOUR, 0, 255) );   /* rouge augmente */
    analogWrite(LEDG, map(opentime, 0, LASTHOUR, 180, 0) );   /*  vert diminue */
    LEDBOff;
  }

  /****************************
     Write state to the galva
   ****************************/
  analogWrite(GALV1, stateGalv1);
  analogWrite(GALV2, stateGalv2);

  /****************************
     Update status
   ****************************/
  minute = opentime;
  ppl = ppl_count / 10;
  hour = minute / 60;
  
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > t_interval) {
    previousMillis = currentMillis;  
    updateStatus();
  }

}

void update_buttons() {
  /*   update debounce buttons  */
  cancel.update();
  levRR.update();
  levRL.update();
  levLR.update();
  levLL.update();
}

/* Is it some light detected in space ? */
boolean updateStatus() {

  String open_closed;
  String txt_open_closed;
  String txt_ppl;

  if (minute == 0) {
    open_closed = "closed";
    txt_open_closed = "The lab is closed.";
  }
  else if (minute > 0 and minute <= 60) {
    open_closed = "open";
    txt_open_closed = "The lab is open - remaining time is " + String(minute) + " min.";
  }
  else {
    open_closed = "open";
    txt_open_closed = "The lab is open - planned to be open for " + String(hour) + " more hours.";
  }

  if (ppl == 0) {
    txt_ppl = "Nobody here !";
  }
  else if (ppl == 1) {
    txt_ppl = "One lonely hacker in the space.";
  }
  else {
    txt_ppl = "There are " + String(ppl) + " hacker in the space.";
  }

  String PostData = "api_key=SECRET=";
  PostData+=txt_open_closed;
  PostData+= " ";
  PostData+= txt_ppl; 
  PostData+= "  [Set by PTL control panel]";
  PostData+="&amp;open_closed=";
  PostData+= open_closed;
  PostData+="&amp;submit=Submit";

  if (client.connect("posttenebraslab.ch", 80)) {
    Serial.println("connected to posttenebraslab.ch");

    Serial.println(PostData);

    // Make a HTTP request:
    client.println("POST /status/change_status HTTP/1.0");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
    Serial.println("disconnecting.");
    client.stop();
  }
  else {
    // If no connection to the server:
    Serial.println("connection failed");
  }
}

/* Is it some light detected in space ? */
/* boolean light() {

  int sensorValue = analogRead(LIGHTPIN);

  if ( sensorValue > LIGHTTHRESOLD )
    return true;
  else
    return false;

}
*/
