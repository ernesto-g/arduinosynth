#include <HardwareSerial.h>
#include <Arduino.h>
#include "Config.h"
#include "MidiManager.h"

const unsigned short NOTES_TABLE_PWM[61] = {1003,990,975,960,944,927,909,891,871,849,827,803,778,753,723,693,661,627,592,554,514,471,426,379};

MidiInfo midiInfo;
byte midiStateMachine=MIDI_STATE_IDLE;
char keysActivatedCounter=0;

void midi_analizeMidiInfo(MidiInfo * pMidiInfo);


void midi_init(void)
{
  midiStateMachine=MIDI_STATE_IDLE;
  keysActivatedCounter=0;  
}

void midi_analizeMidiInfo(MidiInfo * pMidiInfo)
{
    
    if(pMidiInfo->channel==MIDI_CURRENT_CHANNEL)
    {
        if(pMidiInfo->cmd==MIDI_CMD_NOTE_ON)
        {
            // NOTE ON 
            //Serial.print("\r\nNOTE ON:");
            //Serial.print(pMidiInfo->note,DEC);
            //Serial.print(" vel:");            
            //Serial.print(pMidiInfo->vel,DEC);

            if(pMidiInfo->note>=36 && pMidiInfo->note<96)
            {
              unsigned short pwmVal = NOTES_TABLE_PWM[pMidiInfo->note-36];
              if(pMidiInfo->note<60)
              {
                digitalWrite(PIN_VCO1_SCALE, LOW);
                digitalWrite(PIN_VCO2_SCALE, LOW);
              }
              else
              {
                digitalWrite(PIN_VCO1_SCALE, HIGH);
                digitalWrite(PIN_VCO2_SCALE, HIGH);                
              }
              
              OCR1A = pwmVal;    
              OCR1B = pwmVal;  
              //Serial.print("PWM:");
              //Serial.print(pwmVal,DEC);   
              digitalWrite(PIN_GATE_SIGNAL,HIGH);
              keysActivatedCounter++;
            }
            //Serial.print("\r\n");    
        }
        else if(pMidiInfo->cmd==MIDI_CMD_NOTE_OFF)
        {
          // NOTE OFF
            //Serial.print("\r\nNOTE OF:");
            //Serial.print(pMidiInfo->note,DEC);
            //Serial.print(" vel:");            
            //Serial.print(pMidiInfo->vel,DEC);
            //Serial.print("\r\n");
            if(keysActivatedCounter>0)            
              keysActivatedCounter--;
            if(keysActivatedCounter==0)
              digitalWrite(PIN_GATE_SIGNAL,LOW);
        }
        else
        {
          // unsuported command
          
        }
    }     
}


void midi_stateMachine(byte midiByte)
{
  switch(midiStateMachine)
  {
      case MIDI_STATE_IDLE:
        {
        // read status byte 
        midiInfo.channel = midiByte & B00001111;
        midiInfo.cmd = midiByte & B11110000;
        midiStateMachine = MIDI_STATE_RVC_DATA1;
        break;
      }
      case MIDI_STATE_RVC_DATA1:
      {
        // read note
        midiInfo.note = midiByte;
        midiStateMachine = MIDI_STATE_RVC_DATA2;        
        break;
      }
      case MIDI_STATE_RVC_DATA2:
      {
        // read velocity
        midiInfo.vel = midiByte;
        midiStateMachine = MIDI_STATE_IDLE;
        midi_analizeMidiInfo(&midiInfo);
        break;
      }
  }
  
}
