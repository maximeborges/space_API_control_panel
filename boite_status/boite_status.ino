
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
#include <Button.h>

/* PINOUT */
/* Output pins are all PWM pins */
#define GALV1 5
#define GALV2 6
#define LED1 9
#define LEDR 10
#define LEDB 11
#define LEDG 13

/* Input pins are digital pins */
#define OnOff 2
#define LEVRR 4
#define LEVRL 7
#define LEVLR 8
#define LEVLL 12

/* les levier et switch utilisent la librairie Button.h  */
Button cancelSwitch = Button(OnOff, PULLUP);
Button leverRR = Button(LEVRR, PULLUP);
Button leverRL = Button(LEVRL, PULLUP);
Button leverLR = Button(LEVLR, PULLUP);
Button leverLL = Button(LEVLL, PULLUP);

double stateGalva1 = 0;
int stateGalva2 = 0;
int stateLeverRR = 0;
int stateLeverRL = 0;
int stateLeverLR = 0;
int stateLeverLL = 0;
int timeLeverRR, actionTimeLeverRR;

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
  analogWrite(LED1, LOW);
  pinMode(LEDR, OUTPUT);      /* LED rouge sous la coupole blanche */
  analogWrite(LEDR, LOW);
  pinMode(LEDB, OUTPUT);     /* LED bleue sous la coupole blanche */
  analogWrite(LEDB, LOW);
  pinMode(LEDG, OUTPUT);    /* LED verte sous la coupole blanche */
  analogWrite(LEDG, LOW);

}


void loop() {

  if( leverRR.uniquePress() )
  {
    actionTimeLeverRR = millis();
    timeLeverRR = 0;
  }
  while( leverRR.isPressed() && stateGalva1 < 255 )
  {
    timeLeverRR += (millis() - actionTimeLeverRR)/5000;
    stateGalva1 = map(timeLeverRR, 0, 1, 0, 255);
    Serial.println( stateGalva1); 
  }

}


