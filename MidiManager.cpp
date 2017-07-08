#include <HardwareSerial.h>
#include <Arduino.h>
#include "Config.h"
#include "MidiManager.h"
#include "AnalogIns.h"
#include "Outs.h"

const unsigned short NOTES_TABLE_PWM[61+5] = {953,942,936,926,916,903,893,882,866,850,838,822,810,790,772,753,734,707,685,659,635,611,580,550,512,475,446,405,366,316,270,929,919,909,900,888,873,
859,844,831,815,796,777,760,738,717,696,670,648,620,590,560,530,493,457,420,382,335,290,245,200,160,120,80,40,0};
// last valid pwm value: 200

MidiInfo midiInfo;
byte midiStateMachine=MIDI_STATE_IDLE;
char keysActivatedCounter=0;
unsigned char voicesMode;

void midi_analizeMidiInfo(MidiInfo * pMidiInfo);
static unsigned char changeOctave(unsigned char currentOctave, unsigned char noteNumber);
static unsigned short changeTune(signed int currentTune,unsigned char noteNumber,unsigned char* pScale);
static void showMode(void);


static unsigned char currentOctaveVco1;
static unsigned char currentOctaveVco2;
static signed int currentTuneVco1;
static signed int currentTuneVco2;
static unsigned int currentRepeatValue;
static unsigned char repeatRunning;


volatile unsigned int repeatCounter; // incremented in lfo interrupt

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

  repeatCounter = 0;
  repeatRunning=0;

  showMode();
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
              unsigned char noteNumberVco1;
              unsigned char noteNumberVco2;

              // change octave
              noteNumberVco1 = changeOctave(currentOctaveVco1,pMidiInfo->note);
              noteNumberVco2 = changeOctave(currentOctaveVco2,pMidiInfo->note);
              //______________
              
              unsigned short pwmValVco1;
              unsigned short pwmValVco2;
              unsigned char scaleVco1;
              unsigned char scaleVco2;              

              // change tune
              pwmValVco1 = changeTune(currentTuneVco1,noteNumberVco1,&scaleVco1);
              pwmValVco2 = changeTune(currentTuneVco2,noteNumberVco2,&scaleVco2);              
              //____________

              if(voicesMode==MIDI_VOICES_MODE_MONO)
              { 
                // Single voice mode           
                if(scaleVco1==0)
                {
                  digitalWrite(PIN_VCO1_SCALE, LOW);
                }
                else
                {
                  digitalWrite(PIN_VCO1_SCALE, HIGH);
                }
                if(scaleVco2==0)                
                {
                  digitalWrite(PIN_VCO2_SCALE, LOW);
                }
                else
                {
                  digitalWrite(PIN_VCO2_SCALE, HIGH);                
                }
                OCR1B = pwmValVco1;    
                OCR1A = pwmValVco2;  
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

void midi_repeatManager(void)
{
  if(currentRepeatValue>0)
  {    
    if(repeatCounter>=currentRepeatValue)
    {
        repeatCounter=0;
        digitalWrite(PIN_TRIGGER_SIGNAL,HIGH); // trigger=1
        digitalWrite(PIN_GATE_SIGNAL,LOW); // gate=1
        outs_set(OUT_REPEAT,1);
        repeatRunning=1;
    }
    else
    {
      if(repeatRunning==1 && repeatCounter>=(4 + (currentRepeatValue/8) ) ) // wait (100ms + a proportional time) to disable trigger and gate
      {
        digitalWrite(PIN_TRIGGER_SIGNAL,LOW); // trigger=0
        outs_set(OUT_REPEAT,0);      
        repeatRunning=0;
        if(keysActivatedCounter==0)
            digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0
      }
    }
  }
  else if(repeatRunning==1)
  {
        digitalWrite(PIN_TRIGGER_SIGNAL,LOW); // trigger=0
        outs_set(OUT_REPEAT,0);      
        repeatRunning=0;
        if(keysActivatedCounter==0)
            digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0    
  }

}


void midi_setOctaveVco1(byte octave)
{
  currentOctaveVco1 = octave;    
}
void midi_setOctaveVco2(byte octave)
{
    currentOctaveVco2 = octave;
  
}

void midi_setTuneVco1(signed int tuneValue)
{
    currentTuneVco1 = tuneValue;
}
void midi_setTuneVco2(signed int tuneValue)
{
    currentTuneVco2 = tuneValue;
}

void midi_setRepeatValue(unsigned int repeatVal)
{
  if(repeatVal<100)
  {
      currentRepeatValue=0;  
  }
  else
  {
      repeatVal = 1023 - repeatVal; // invert value
      currentRepeatValue= (repeatVal+30)/6 ; // currentRepeatValue between 5 and 158 (125ms to 3.9s)
  }
}

static unsigned char changeOctave(unsigned char currentOctave, unsigned char noteNumber)
{
    switch(currentOctave)
    {
        case OCTAVE_MINUS_TWO:
          if( noteNumber>=(36+24) )
            noteNumber-=24;
          break;
        case OCTAVE_MINUS_ONE:
          if( noteNumber>=(36+12) )
            noteNumber-=12;
          break;                    
        case OCTAVE_PLUS_ONE:
          if( noteNumber<=(96-12) )
            noteNumber+=12;
          break;
        case OCTAVE_PLUS_TWO:
          if( noteNumber<=(96-24) )
            noteNumber+=24;
          break;                    
    }
    return noteNumber;
}


static unsigned short changeTune(signed int currentTune,unsigned char noteNumber,unsigned char* pScale)
{
      if(noteNumber<=66)
        *pScale=0;
      else
        *pScale=1;
      
      unsigned char n = noteNumber-36;
      signed long pwmP4;// =  NOTES_TABLE_PWM[n+4];
      signed long pwm0; // =  NOTES_TABLE_PWM[n];

      if(n<=26 || n>=31 ) // same scale. calculate delta for 4 notes deviation
      {
        pwmP4 =  NOTES_TABLE_PWM[n+4];
        pwm0 =  NOTES_TABLE_PWM[n];
      }
      else
      { // the note is in the middle of the pwm scale, adjacent delta is used
        pwmP4 =  NOTES_TABLE_PWM[30];
        pwm0 =  NOTES_TABLE_PWM[26];        
      }
      return  ((unsigned short)( ( ((signed long)currentTune) * (pwmP4 - pwm0)) / 512 )) + NOTES_TABLE_PWM[n];      
}

static void showMode(void)
{
    outs_set(OUT_MODE0,0);
    outs_set(OUT_MODE1,0);
    outs_set(OUT_MODE2,0);
    outs_set(OUT_MODE3,0);

    switch(voicesMode)
    {
      case 0:
        outs_set(OUT_MODE0,1);
        break;
      case 1:
        outs_set(OUT_MODE1,1);
        break;
      case 2:
        outs_set(OUT_MODE2,1);
        break;
      case 3:
        outs_set(OUT_MODE3,1);
        break;
    }  
}

