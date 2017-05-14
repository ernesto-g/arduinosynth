
#include <Arduino.h>
#include <HardwareSerial.h>
#include "AnalogIns.h"

static uint16_t inputsValue[6];
static uint8_t currentChn;
static uint8_t state;

void ain_init(void)
{
  currentChn=0;
  analogRead(0); // read for initialization
  state = ANALOG_STATE_IDLE;
}

void ain_state_machine(void)
{
    switch(state)
    {
      case ANALOG_STATE_IDLE:
      {
        // set channel
        ADMUX = ADMUX & 0xF0;
        ADMUX = ADMUX | (currentChn & 0x0F);
        state = ANALOG_STATE_START;
        break;  
      }
      case ANALOG_STATE_START:
      {
        // Start conversion
        ADCSRA|=0x40;
        state = ANALOG_STATE_WAIT;
        break;
      }
      case ANALOG_STATE_WAIT:
      {
        if(!bit_is_set(ADCSRA, ADSC))
        {
          uint8_t low, high;
          low  = ADCL;
          high = ADCH;
          inputsValue[currentChn] = (high << 8) | low;
          state = ANALOG_STATE_FINISH;
        }
        break;
      }
      case ANALOG_STATE_FINISH:
      {
        //Serial.print("Valor entrada ");
        //Serial.print(currentChn,DEC);
        //Serial.print(": ");
        //Serial.print(inputsValue[currentChn],DEC);
        //Serial.print("\r\n");
                
        currentChn++;
        if(currentChn>=6)
          currentChn=0;
        state = ANALOG_STATE_IDLE;  
        break;
      }
    }
}


