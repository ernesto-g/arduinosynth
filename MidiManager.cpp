#include <HardwareSerial.h>
#include <Arduino.h>
#include "Config.h"
#include "MidiManager.h"

const unsigned short NOTES_TABLE_PWM[61] = {953,942,936,926,916,903,893,882,866,850,838,822,810,790,772,753,734,707,685,659,635,611,580,550,512,475,446,405,366,316,270,929,919,909,900,888,873,
859,844,831,815,796,777,760,738,717,696,670,648,620,590,560,530,493,457,420,382,335,290,245,200};

MidiInfo midiInfo;
byte midiStateMachine=MIDI_STATE_IDLE;
char keysActivatedCounter=0;
unsigned char voicesMode;

void midi_analizeMidiInfo(MidiInfo * pMidiInfo);


void midi_init(void)
{
  midiStateMachine=MIDI_STATE_IDLE;
  keysActivatedCounter=0;  

  voicesMode = MIDI_VOICES_MODE_MONO;
  
  digitalWrite(PIN_VCO1_SCALE, HIGH);
  digitalWrite(PIN_VCO2_SCALE, HIGH);
  digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0
  unsigned short pwmVal = NOTES_TABLE_PWM[30];
  OCR1A = pwmVal;    
  OCR1B = pwmVal;  
}

void midi_analizeMidiInfo(MidiInfo * pMidiInfo)
{
    if(pMidiInfo->channel==MIDI_CURRENT_CHANNEL)
    {
        if(pMidiInfo->cmd==MIDI_CMD_NOTE_ON)
        {
            // NOTE ON 
            if(pMidiInfo->note>=36 && pMidiInfo->note<=96)
            {
              digitalWrite(PIN_TRIGGER_SIGNAL,HIGH); // trigger=1
              digitalWrite(PIN_GATE_SIGNAL,LOW); // gate=1
              
              keysActivatedCounter++;
              unsigned char noteNumberVco1 = pMidiInfo->note;
              unsigned char noteNumberVco2 = pMidiInfo->note;
              
              unsigned short pwmValVco1 = NOTES_TABLE_PWM[noteNumberVco1-36];
              unsigned short pwmValVco2 = NOTES_TABLE_PWM[noteNumberVco2-36];

              if(voicesMode==MIDI_VOICES_MODE_MONO)
              { 
                // Single voice mode           
                if(noteNumberVco1<=66)
                {
                  digitalWrite(PIN_VCO1_SCALE, LOW);
                }
                else
                {
                  digitalWrite(PIN_VCO1_SCALE, HIGH);
                }
                if(noteNumberVco2<=66)
                {
                  digitalWrite(PIN_VCO2_SCALE, LOW);
                }
                else
                {
                  digitalWrite(PIN_VCO2_SCALE, HIGH);                
                }
                OCR1A = pwmValVco1;    
                OCR1B = pwmValVco2;  
              }
              else
              {
                // Two voices mode
                
              }
              
              
              digitalWrite(PIN_TRIGGER_SIGNAL,LOW); // trigger=0
            }
        }
        else if(pMidiInfo->cmd==MIDI_CMD_NOTE_OFF)
        {
          // NOTE OFF
          if(keysActivatedCounter>0)            
              keysActivatedCounter--;
          if(keysActivatedCounter==0)
              digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0
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


void midi_setOctaveVco1(byte octave)
{
  
}
void midi_setOctaveVco2(byte octave)
{
  
}

void midi_setTuneVco1(signed int tuneValue)
{
  
}
void midi_setTuneVco2(signed int tuneValue)
{
  
}



