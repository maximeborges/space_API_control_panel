
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
#define STEP 2               /* step de l'acceleration aiguille step+step*temps)  */
#define MINUTE 60000         /* Vitesse de descente de l'aiguille (60 secondes dans une minute)  */
#define GALV1RANGE 540       /* Le range du galva 1 540 == 9*60 min - 9 heures */
#define GALV2RANGE 300       /* Le range du galva 2 300 (fonction à determiner ) */
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
long galv2scale = 0;

unsigned long reftime = 0;
unsigned long ledtime = 0;     /*  timer pour acceleration aiguille galva */


void setup() {

 // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only

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
    galv2scale = constrain( galv2scale + STEP + STEP * (long)levLR.duration()/500, 0 , GALV2RANGE );  /* acceleration, valeur de 0 à GALV2RANGE */
    delay(35);
  }
  else if( !levLL.read() )
  {
    galv2scale = constrain( galv2scale - STEP -  STEP * (long)levLL.duration()/500, 0 , GALV2RANGE );  /* acceleration, valeur de 0 à GALV2RANGE */
    delay(35);
  }


  /********************
  * Interrupteur
  *********************/
  if( cancel.read() && ( cancel.duration() > 2000 ) )
  {
    opentime = 0;
    galv2scale = 0;
  }

  stateGalv1 = map(opentime, 0, GALV1RANGE, 0, 255);        /*  durée d'ouverture de 0 à 540 min <- stateGalv1(0,255)  */
  stateGalv2 = map(galv2scale, 0, GALV2RANGE, 0, 255);      /*  durée d'ouverture de 0 à 540 min <- stateGalv1(0,255)  */


  /*  Une LED rouge clignotte lentement quand le local est fermé */
  if( !opentime )
  {
    if( (millis()-ledtime)> ( BLINKPERIOD + BLINKOn ) )
    {
      LED1Off;
      ledtime = millis();       /*  remise à zero du compteur  */

    }else if( (millis()-ledtime)> BLINKPERIOD ) {
      LED1On;
    }
  }else{
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

    /**********************************
     * Output for the server
     **********************************/
    Serial.print("Galva 1 ; ");    /*  for the server  */
    Serial.print( opentime );
    Serial.print("; Gavla 2 ; ");
    Serial.println( galv2scale );

  }


  if( opentime == 0 )               /*  la dernière heure on passe du vert au rouge  */
  {

    LEDROff;
    LEDBOff;
    LEDGOff;

  }else if( opentime > 60 ) {       /* quand plus d'une heure c'est vert */

    LEDROff;
    LEDBOff;
    LEDGOn;

  }else {                            /*  Entre LASTHOUR et 0 on passe de Green à Red */
 
    analogWrite(LEDR, map(opentime, 0, LASTHOUR, 0, 255) );   /* rouge augmente */
    analogWrite(LEDG, map(opentime, 0, LASTHOUR, 180, 0) );        /*  vert diminue */
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
