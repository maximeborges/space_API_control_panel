
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
#define galva1Pin 5
#define galva2Pin 6
#define ledPin 9
#define redPin 10
#define bluePin 11
#define greenPin 13

/* Input pins are digital pins */
#define switchPin 2
#define leverRRPin 4
#define leverRLPin 7
#define leverLRPin 8
#define leverLLPin 12

/* les levier et switch utilisent la librairie Button.h  */
Button cancelSwitch = Button(switchPin, PULLUP);
Button leverRR = Button(leverRRPin, PULLUP);
Button leverRL = Button(leverRLPin, PULLUP);
Button leverLR = Button(leverLRPin, PULLUP);
Button leverLL = Button(leverLLPin, PULLUP);

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

  pinMode(switchPin, INPUT);
  pinMode(leverRRPin, INPUT);
  pinMode(leverRLPin, INPUT);
  pinMode(leverLRPin, INPUT);
  pinMode(leverLLPin, INPUT);
  
  pinMode(ledPin, OUTPUT);      /* LED sous la coupole rouge (elle est blanche) */
  analogWrite(ledPin, LOW);
  pinMode(redPin, OUTPUT);      /* LED rouge sous la coupole blanche */
  analogWrite(redPin, LOW);
  pinMode(bluePin, OUTPUT);     /* LED bleue sous la coupole blanche */
  analogWrite(bluePin, LOW);
  pinMode(greenPin, OUTPUT);    /* LED verte sous la coupole blanche */
  analogWrite(greenPin, LOW);

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


