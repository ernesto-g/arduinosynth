/**
 *  Arduino Analog-Digital Synth
    Copyright (C) <2017>  Ernesto Gigliotti <ernestogigliotti@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <HardwareSerial.h>
#include <Arduino.h>
#include "Config.h"
 
void config_init(void)
{
  
  // Configure serial port for MIDI input
  Serial.begin(31250); 
  //_____________________________________


  // Configure TIMER1 for PWM (VCO1 and VCO2 Control Voltage)
  DDRB |= (1 << DDB1) | (1 << DDB2);
  TCCR1A =
      (1 << COM1A1) | (1 << COM1B1) |
      // Fast PWM mode.
      (1 << WGM11);
  TCCR1B =
      // Fast PWM mode.
      (1 << WGM12) | (1 << WGM13) |
      // No clock prescaling (fastest possible
      // freq).
      (1 << CS10);
  OCR1A = 0;
  OCR1B = 0;
  ICR1 = 1024;
  //__________________________


  // Configure TIMER2 for PWM (LFO output)
  DDRD |= (1 << DDD3);
  TCCR2A =
      (1 << COM2B1) |
      // Fast PWM mode.
      (1 << WGM20) | (1 << WGM21);
  TCCR2B =
      // No clock prescaling (fastest possible
      // freq).
      (1 << CS20);
  OCR2A = 0;
  OCR2B = 0;
  //ICR2 = 255;
  //__________________________
  
  // Configure TIMER0 for DDS interrupt
  cli();//stop interrupts
  // each 133uS 
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 150; //31; // 133uS
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();//allow interrupts
  //___________________________________
  

  // Init Pins
  pinMode(PIN_VCO1_SCALE,OUTPUT);
  pinMode(PIN_VCO2_SCALE,OUTPUT);
  pinMode(PIN_GATE_SIGNAL,OUTPUT);
  pinMode(PIN_TRIGGER_SIGNAL,OUTPUT);
  digitalWrite(PIN_GATE_SIGNAL,HIGH);
  digitalWrite(PIN_TRIGGER_SIGNAL,LOW);

  pinMode(PIN_OUTS_DATA,OUTPUT);
  pinMode(PIN_OUTS_CLK,OUTPUT);
  pinMode(PIN_OUTS_STROBE,OUTPUT);

  pinMode(PIN_BUTTON,INPUT);
  pinMode(PIN_GLISS_SWITCH,INPUT);

  //__________
   
 }

