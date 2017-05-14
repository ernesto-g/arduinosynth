#include <HardwareSerial.h>
#include <Arduino.h>
#include "Config.h"
#include "Lfo.h"

const unsigned char PROGMEM SINETABLE[10] = {0,1,2,3,4,5,6,7,8};
const unsigned char PROGMEM SAWTOOTHTABLE[10] = {0,1,2,3,4,5,6,7,8};

unsigned char toggle=0;
ISR(TIMER0_COMPA_vect)
{
  if(toggle==0)
  {
    toggle=1;
  }
  else
  {
    toggle=0;
  }
}



void lfo_init(void)
{
  
}

