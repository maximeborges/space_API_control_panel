/*
 * =====================================================================================
 *
 *       Filename:  boite_status.ino
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/05/2014
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Sebastien Chassot (sinux), seba.ptl@sinux.net
 *       Modified:  Alexandre Rosenberg
 *        Company:  PostTenebrasLab the geneva's hackerspace   posttenebraslab.ch
 *
 * =====================================================================================
 *   Alexandre Rosenberg - Added basic serial control (speed: 115200)
 *   "get 1", "get 2" for galva 1 and 2 value
 *   "set 1 x" to set galva 1 to value x, "set 2 x" for galva 2
 *
 *   Alexandre Rosenberg - tweaked value and scale to non linear
 *   to match with the printed design.
 *
 */

#include <SPI.h>
#include <Bounce.h>

/* Founctions  */
void update_buttons();       /* debounce lib need an update every loop */

/* PINOUT */
/* Output pins are all PWM pins */
#define GALV1 5              /*  Galva 1 (gauche)  */
#define GALV2 6              /*  Galva 2 (droite)  */
#define LED1 9               /*  Led du haut Red */
#define LEDR 10              /*  Led du bas Red  */
#define LEDB 11              /*  Led du bas Blue  */
#define LEDG 13              /*  Led du bas Green  */

/* Input pins are digital pins */
#define OnOff 2              /* Interupteur du millieu On/Off */
#define LEVRR 7              /* Levier Right position Right */
#define LEVRL 4              /* Levier Right position Left */
#define LEVLR 12              /* Levier Left position Right */
#define LEVLL 8             /* Levier Left position Left */

/* Const */
#define STEP 1               /* step de l'acceleration aiguille step+step*temps)  */
#define MINUTE 60000         /* Vitesse de descente de l'aiguille (60 secondes dans une minute)  */
#define GALV1RANGE 960       /* Le range du galva 1 960 == 16*60 min - 16 heures */
#define GALV2RANGE 160       /* Le range du galva 2 160. max 16 ppl ) */
#define BLINKPERIOD 3000     /* Frequence de cligotement du temoin rouge (local fermé) */
#define BLINKOn 500          /* durée de clignotement du temoin */
#define LASTHOUR 60          /* en se raprochant de 0 la LED va progressivement changer de couleur à partir de X minutes */

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

unsigned long reftime = 0;
unsigned long ledtime = 0;     /*  timer pour acceleration aiguille galva */

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

//used for serial command
int galva_no = 0;
int new_value = 0;

void setup() {

  // Open serial communications and wait for port to open:
  // initialize serial:
  Serial.begin(115200);
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

  update_buttons();

  /********************
   * levier 1
   *********************/
  if( !levRR.read() )
  {
    opentime = constrain( opentime + STEP + STEP * (long)levRR.duration()/500, 0 , GALV1RANGE );  /* acceleration, valeur de 0 à GALV1RANGE */
    reftime = millis();
    delay(35);
  }
  else if( !levRL.read() )
  {
    opentime = constrain( opentime - STEP -  STEP * (long)levRL.duration()/500, 0 , GALV1RANGE );  /* acceleration, valeur de 0 à GALV1RANGE */
    delay(35);
  }


  /********************
   * levier 2
   *********************/
  if( !levLR.read() )
  {
    ppl_count = constrain( ppl_count + STEP + STEP * (long)levLR.duration()/500, 0 , GALV2RANGE );  /* acceleration, valeur de 0 à GALV2RANGE */
    delay(35);
  }
  else if( !levLL.read() )
  {
    ppl_count = constrain( ppl_count - STEP -  STEP * (long)levLL.duration()/500, 0 , GALV2RANGE );  /* acceleration, valeur de 0 à GALV2RANGE */
    delay(35);
  }


  /********************
   * Interrupteur
   *********************/
  if( cancel.read() && ( cancel.duration() > 2000 ) )
  {
    opentime = 0;
    ppl_count = 0;
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
  if( !opentime )
  {
    if( (millis()-ledtime)> ( BLINKPERIOD + BLINKOn ) )
    {
      LED1Off;
      ledtime = millis();       /*  remise à zero du compteur  */

    }
    else if( (millis()-ledtime)> BLINKPERIOD ) {
      LED1On;
    }
  }
  else{
    LED1Off;
  }

  /****************************************************
   *
   * Temps 
   *
   ***************************************************/

  /*  toute les minutes on decremente le temps */
  if( (millis() - reftime) >= MINUTE )
  {
    opentime--;

    if(opentime < 0)
      opentime = 0;
    reftime = millis();
    // Serial.println("timer decrement");
  }

  if( opentime != old_opentime )
  {
    old_opentime = opentime;
  }
  /**********************************
   * Serial output for the server
   **********************************/
  // check string send by serial:
  if (stringComplete) {
    if (inputString == "get 1\n")
    {
      // print data  
      Serial.println( opentime ); // Valeur galva 1
    }
    else if (inputString == "get 2\n")
    {
      Serial.println( ppl_count ); // Valeur galva 2
    }
    else if (inputString.startsWith("set"))
    {
      galva_no = getValue(inputString, ' ', 1).toInt();
      new_value = getValue(inputString, ' ', 2).toInt();

      if (galva_no == 1)
      {
        opentime = new_value;
      }
      else if (galva_no == 2)
      {
        ppl_count = new_value;
      }  
    }
    else
    {
      Serial.println("unknown command");
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  if( opentime == 0 )               /*  la dernière heure on passe du vert au rouge  */
  {

    LEDROff;
    LEDBOff;
    LEDGOff;

  }
  else if( opentime > 180 ) {       /* quand plus d'une heure et demi c'est vert */

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
   * Write state to the galva
   ****************************/
  analogWrite(GALV1, stateGalv1);
  analogWrite(GALV2, stateGalv2);

}


void update_buttons() {
  /*   update debounce buttons  */
  cancel.update();
  levRR.update();
  levRL.update();
  levLR.update();
  levLL.update();

}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

// From http://stackoverflow.com/questions/9072320/split-string-into-string-array?utm_source=twitterfeed&utm_medium=twitter
// This function returns a single string separated by a predefined character at a given index.
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1  };
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

