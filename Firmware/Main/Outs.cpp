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
#include <Arduino.h>
#include "Config.h"

#define OUTS_STATE_IDLE         0
#define OUTS_STATE_SENDING      1
#define OUTS_STATE_GENERATE_CLK 2
#define OUTS_STATE_FINISHED     3


static unsigned char outsValue;
static unsigned char flagStartSend;
static unsigned char state;
static unsigned char index;

void outs_init(void)
{
  outsValue=0x00;
  flagStartSend=1;
}


void outs_stateMachine(void)
{
  switch(state)
  {
    case OUTS_STATE_IDLE:
    {
        if(flagStartSend==1)
        {
          flagStartSend = 0;
          state = OUTS_STATE_SENDING;
          index = 0;
          digitalWrite(PIN_OUTS_STROBE, LOW);
        }
        break;
    }
    case OUTS_STATE_SENDING:
    {
      if( (outsValue & (0x80>>index))==0x00 )
      {
            digitalWrite(PIN_OUTS_DATA, LOW);
      }
      else
      {              
            digitalWrite(PIN_OUTS_DATA, HIGH);
      }
      state = OUTS_STATE_GENERATE_CLK;
      break;
    }
    case OUTS_STATE_GENERATE_CLK:
    {
      digitalWrite(PIN_OUTS_CLK, HIGH);
      index++;
      digitalWrite(PIN_OUTS_CLK, LOW);
      
      if(index==8)
        state = OUTS_STATE_FINISHED;
      else
        state = OUTS_STATE_SENDING;
      break;
    }
    case OUTS_STATE_FINISHED:
    {
      digitalWrite(PIN_OUTS_STROBE, HIGH);
      state = OUTS_STATE_IDLE;
      break;
    }
    
  }
  
}

void outs_set(unsigned char number,unsigned char value)
{
    if(value==0)
    {
        outsValue&= (~(1<<number));        
    }
    else
    {
        outsValue|= (1<<number);              
    }
    flagStartSend=1;
}

void outs_updateOuts(void)
{
  flagStartSend=1;
}

