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
#include "MidiManager.h"
#include "AnalogIns.h"
#include "Outs.h"
#include "Lfo.h"
#include "SequencerManager.h"

const unsigned short NOTES_TABLE_PWM[61+5] = {953,942,936,926,916,903,893,882,866,850,838,822,810,790,772,753,734,707,685,659,635,611,580,550,512,475,446,405,366,316,270,929,919,909,900,888,873,
859,844,831,815,796,777,760,738,717,696,670,648,620,590,560,530,493,457,420,382,335,290,245,200,160,120,80,40,0};
// last valid pwm value: 200

MidiInfo midiInfo;
byte midiStateMachine=MIDI_STATE_IDLE;
unsigned char voicesMode;

void midi_analizeMidiInfo(MidiInfo * pMidiInfo);
static unsigned char changeOctave(unsigned char currentOctave, unsigned char noteNumber);
static unsigned short changeTune(signed int currentTune,unsigned char noteNumber,unsigned char* pScale);
static void showMode(void);
static byte saveKey(byte note);
static byte getIndexOfPressedKey(byte note);
static byte deleteKey(byte note);
static byte thereAreNoKeysPressed(void);
static byte getTheLowestKeyPressed(void);
static byte getTheHighestKeyPressed(void);
static void setVCOs(byte note);
static byte getNextKeyForRepeat(void);


static unsigned char currentOctaveVco1;
static unsigned char currentOctaveVco2;
static signed int currentTuneVco1;
static signed int currentTuneVco2;
static unsigned int currentRepeatValue;
static unsigned char repeatRunning;
static unsigned char repeatOn;
static unsigned char lfoIsSynced;
static unsigned char repeatKeyIndex;

static unsigned char isGlissOn;
static unsigned int glissSpeed;
static unsigned char glissFinalKey;
static unsigned char glissStartKey;
static unsigned int glissState;


volatile unsigned int repeatCounter; // incremented in lfo interrupt
volatile unsigned int glissCounter; // incremented in lfo interrupt

static KeyPressedInfo keysPressed[KEYS_PRESSED_LEN];

void midi_init(void)
{
  byte i;
  midiStateMachine=MIDI_STATE_IDLE;

  for(i=0; i<KEYS_PRESSED_LEN; i++)
    keysPressed[i].flagFree=1;

  repeatKeyIndex=0;
  voicesMode = MIDI_MODE_MONO_KEYS_BOTH_SIDES;
  
  digitalWrite(PIN_VCO1_SCALE, HIGH);
  digitalWrite(PIN_VCO2_SCALE, HIGH);
  digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0
  unsigned short pwmVal = NOTES_TABLE_PWM[30];
  OCR1A = pwmVal;    
  OCR1B = pwmVal;  

  repeatCounter = 0;
  repeatRunning=0;
  repeatOn=0;
  lfoIsSynced = 0;
  isGlissOn = 0;
  glissSpeed = 0;
  glissFinalKey = 0xFF;
  glissStartKey = 0xFF;
  glissState = GLISS_STATE_IDLE;
  showMode();
}

/*
  MODE 0 : No key priority: last key pressed is valid
  MODE 1 : Lower key priority: key is valid if it has lower freq than last one
  MODE 2 : 2 voices mode:  The lowest key is played on VCO2. The highest key is played on VCO1
  MODE 3 : Sequence mode. Record and play sequence.   
*/
void midi_analizeMidiInfo(MidiInfo * pMidiInfo)
{
    if(pMidiInfo->channel==MIDI_CURRENT_CHANNEL)
    {
        if(pMidiInfo->cmd==MIDI_CMD_NOTE_ON)
        {
            // NOTE ON 
            if(pMidiInfo->note>=36 && pMidiInfo->note<=96)
            { 

              if(voicesMode==MIDI_MODE_MONO_KEYS_LOW_PRIOR)
              {
                  if(pMidiInfo->note > getTheLowestKeyPressed())
                  {
                    saveKey(pMidiInfo->note);
                    return; // ignore key                         
                  }
              }
              saveKey(pMidiInfo->note);
             
              if(repeatOn==1)
                return; // repeat is playing, save key to repeat later, but ignore current key hit

              if(isGlissOn==1)
              {
                  glissFinalKey = pMidiInfo->note;
                  if(seq_isRecording())
                    seq_startRecordNote(pMidiInfo->note);
                  return; // gliss switch is on, dont play the note now
              }

              digitalWrite(PIN_TRIGGER_SIGNAL,HIGH); // trigger=1
              digitalWrite(PIN_GATE_SIGNAL,LOW); // gate=1

              if(lfoIsSynced)
                lfo_reset();
              if(seq_isRecording())
                seq_startRecordNote(pMidiInfo->note);

              setVCOs(pMidiInfo->note);
              
              digitalWrite(PIN_TRIGGER_SIGNAL,LOW); // trigger=0
            }
        }
        else if(pMidiInfo->cmd==MIDI_CMD_NOTE_OFF)
        {
          // NOTE OFF
          if(deleteKey(pMidiInfo->note))
          {
            if(thereAreNoKeysPressed())
            {
                digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0
            }
            else
            {
              if(voicesMode==MIDI_MODE_MONO_KEYS_LOW_PRIOR)
              {
                // a key was released. keep playing previous lower key
                byte previousNote = getTheLowestKeyPressed();
                if(previousNote!=0xFF)
                  setVCOs(previousNote);
              }
            }
            
            if(seq_isRecording())
              seq_endRecordNote();
          }

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

void midi_startNote(unsigned char midiNoteNumber)
{
    MidiInfo mi;
    mi.cmd = MIDI_CMD_NOTE_ON;
    mi.note = midiNoteNumber;
    mi.channel = MIDI_CURRENT_CHANNEL;
    midi_analizeMidiInfo(&mi);
}
void midi_stopNote(unsigned char midiNoteNumber)
{
    MidiInfo mi;
    mi.cmd = MIDI_CMD_NOTE_OFF;
    mi.note = midiNoteNumber;
    mi.channel = MIDI_CURRENT_CHANNEL;
    midi_analizeMidiInfo(&mi);  
}

void midi_glissManager(void)
{
    if(isGlissOn==0)
    {
        glissFinalKey=0xFF;
        glissStartKey=0xFF;
        return;
    }

    switch(glissState)
    {
        case GLISS_STATE_IDLE:
        {
          if(glissFinalKey!=glissStartKey)
          {
              // Gliss is activated
              glissState = GLISS_STATE_CHANGE_NOTE; //glissSpeed 
          }          
          break;
        }
        case GLISS_STATE_CHANGE_NOTE:
        {
          if(glissFinalKey!=glissStartKey)
          {
              // first key case
              if(glissStartKey==0xFF)
              {
                glissStartKey = glissFinalKey;
              }
              else
              {
                // second key case
                if(glissFinalKey>glissStartKey)
                    glissStartKey++;                  
                else
                    glissStartKey--;
              }
              // play note
              glissCounter=0;
              digitalWrite(PIN_TRIGGER_SIGNAL,HIGH); // trigger=1
              digitalWrite(PIN_GATE_SIGNAL,LOW); // gate=1
              if(lfoIsSynced)
                    lfo_reset();
              setVCOs(glissStartKey);
              outs_set(OUT_REPEAT,1);
              glissState = GLISS_STATE_WAIT_NOTE_DURATION;
          }
          else
            glissState = GLISS_STATE_IDLE;
          break;  
        }
        case GLISS_STATE_WAIT_NOTE_DURATION:
        {
            if(glissCounter>=glissSpeed)
            {
              digitalWrite(PIN_TRIGGER_SIGNAL,LOW); // trigger=0
              outs_set(OUT_REPEAT,0);      
              repeatRunning=0;
              if(thereAreNoKeysPressed())
                  digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0                        

              glissState = GLISS_STATE_CHANGE_NOTE;
            }
          break;
        }
        
    }

}

void midi_repeatManager(void)
{
  if(currentRepeatValue>0)
  {
    repeatOn=1;    
    if(repeatCounter>=currentRepeatValue)
    {
        repeatCounter=0;
        byte note2Play = getNextKeyForRepeat();
        if(note2Play!=0xFF)
        {
          digitalWrite(PIN_TRIGGER_SIGNAL,HIGH); // trigger=1
          digitalWrite(PIN_GATE_SIGNAL,LOW); // gate=1
          if(lfoIsSynced)
                lfo_reset();
          setVCOs(note2Play);
        }
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
        if(thereAreNoKeysPressed())
            digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0
      }
    }
  }
  else 
  {
    repeatOn=0;
    if(repeatRunning==1)
    {
        digitalWrite(PIN_TRIGGER_SIGNAL,LOW); // trigger=0
        outs_set(OUT_REPEAT,0);      
        repeatRunning=0;
        if(thereAreNoKeysPressed())
            digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0    
    }
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
  if(voicesMode==MIDI_MODE_SECUENCER)
  {
    seq_setSpeed((1023-repeatVal)/12);
    currentRepeatValue=0;
  }
  else
  {
      if(isGlissOn)
      {
        glissSpeed = (1023-repeatVal)/12;  
        currentRepeatValue=0;
        return; // repeat is disabled if gliss mode is ON
      }
      
      if(repeatVal<100)
      {
          currentRepeatValue=0;  
      }
      else
      {
          repeatVal = 1023 - repeatVal; // invert value
          currentRepeatValue= (repeatVal+30)/12 ; // currentRepeatValue between 2 and 158 (50ms to 1.9s)
      }
  }
}

void midi_setLfoSync(unsigned int val)
{
    if(val<512)
      lfoIsSynced=0;
    else
      lfoIsSynced=1;
}

void midi_setGlissOn(unsigned char val)
{
    isGlissOn = val;
}

void midi_buttonPressedLongCallback(void)
{
    if(voicesMode==MIDI_MODE_SECUENCER)
    {
        if(seq_isRecording()==0)
        {
          // start recording mode
          seq_startRecord();
        }
        else
        {
          // play mode
          seq_startPlay();
        }
    }
}

void midi_buttonPressedShortCallback(void)
{    
    voicesMode++;
    if(voicesMode>=4)
      voicesMode=0;

    showMode();

    if(voicesMode==MIDI_MODE_SECUENCER)
      seq_startPlay();
    else
    {
        byte i;
        for(i=0; i<KEYS_PRESSED_LEN; i++)
          keysPressed[i].flagFree=1;
        digitalWrite(PIN_GATE_SIGNAL,HIGH); // gate=0  
      seq_stopPlay();
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
      case MIDI_MODE_MONO_KEYS_BOTH_SIDES:
        outs_set(OUT_MODE0,1);
        break;
      case MIDI_MODE_MONO_KEYS_LOW_PRIOR:
        outs_set(OUT_MODE1,1);
        break;
      case MIDI_MODE_DUAL_KEYS_BOTH_SIDES:
        outs_set(OUT_MODE2,1);
        break;
      case MIDI_MODE_SECUENCER:
        outs_set(OUT_MODE3,1);
        break;
    }  
}


static byte saveKey(byte note)
{
  byte i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==1)
    {
        keysPressed[i].flagFree=0;
        keysPressed[i].note = note;
        return 0;
    }
  }
  return 1; // no more space
}
static byte getIndexOfPressedKey(byte note)
{
  byte i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
        if(keysPressed[i].note == note)
          return i;
    }
  }
  return 0xFF; 
}
static byte deleteKey(byte note)
{
    byte index = getIndexOfPressedKey(note);
    if(index<KEYS_PRESSED_LEN)
    {
      keysPressed[index].flagFree=1;
      return 1;
    }
    return 0;
}

static byte thereAreNoKeysPressed(void)
{
  byte i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
      return 0;
    }
  }
  return 1; 
}

static byte getTheLowestKeyPressed(void)
{
    byte i;
    byte mi = 0xFF;
    for(i=0; i<KEYS_PRESSED_LEN; i++)
    {
      if(keysPressed[i].flagFree==0)
      {
          if(keysPressed[i].note<mi)
            mi = keysPressed[i].note;
      }
    }
    return mi;
}

static byte getTheHighestKeyPressed(void)
{
    byte i;
    byte max = 0x00;
    for(i=0; i<KEYS_PRESSED_LEN; i++)
    {
      if(keysPressed[i].flagFree==0)
      {
          if(keysPressed[i].note>max)
            max = keysPressed[i].note;
      }
    }
    return max;
}

static byte getNextKeyForRepeat(void)
{
    byte i;
    byte found=0;
    byte ret=0xFF;
    for(i=repeatKeyIndex; i<KEYS_PRESSED_LEN; i++)
    {
        if(keysPressed[i].flagFree==0)
        {
          ret = keysPressed[i].note;
          found = 1;
          break;
        }
    }
    if(found==0)
    {
        repeatKeyIndex = 0;
        for(i=repeatKeyIndex; i<KEYS_PRESSED_LEN; i++)
        {
            if(keysPressed[i].flagFree==0)
            {
              ret = keysPressed[i].note;
              break;
            }
        }        
    }
    
    repeatKeyIndex = i+1;
    if(repeatKeyIndex>=KEYS_PRESSED_LEN)
      repeatKeyIndex = 0;

   return ret;
}



static void setVCOs(byte note)
{
      unsigned char noteNumberVco1;
      unsigned char noteNumberVco2;
      unsigned short pwmValVco1;
      unsigned short pwmValVco2;
      unsigned char scaleVco1;
      unsigned char scaleVco2;              
      unsigned char note1=note;
      unsigned char note2=note;
      
      if(voicesMode==MIDI_MODE_DUAL_KEYS_BOTH_SIDES)
      {
          note1 = getTheHighestKeyPressed();
          if(note1==0x00)
            note1=note;
            
          note2 = getTheLowestKeyPressed();
          if(note2==0xFF)
              note2=note;  
      }  

      // change octave
      noteNumberVco1 = changeOctave(currentOctaveVco1,note1);
      noteNumberVco2 = changeOctave(currentOctaveVco2,note2);
      //______________
      
      // change tune
      pwmValVco1 = changeTune(currentTuneVco1,noteNumberVco1,&scaleVco1);
      pwmValVco2 = changeTune(currentTuneVco2,noteNumberVco2,&scaleVco2);              
      //____________

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

