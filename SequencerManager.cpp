#include <Arduino.h>
#include "SequencerManager.h"
#include "MidiManager.h"
#include "Outs.h"


struct S_NoteInfo
{
    unsigned char midiNoteNumber;
    unsigned int duration;
};
typedef struct S_NoteInfo NoteInfo;

#define SEQUENCE_LEN  64

#define SEQ_STATE_IDLE                  0
#define SEQ_STATE_PLAY                  1
#define SEQ_STATE_PLAYING               2
#define SEQ_STATE_WAIT_NOTE_DURATION    3
#define SEQ_STATE_WAIT_SILENCE_DURATION 4
#define SEQ_STATE_RECODING              5


static NoteInfo sequence[SEQUENCE_LEN];
static unsigned char recordIndex;
static unsigned char playIndex;
static unsigned char state;
static unsigned int silenceDuration;
static unsigned char isRecording;
static unsigned int counterToggleLed;
static unsigned char ledRec;

volatile unsigned int sequenceNoteDurationCounter; // this variable is incremented in lfo interrupt each 25ms

void seq_init(void)
{
    state = SEQ_STATE_IDLE;
    silenceDuration = 0;
    recordIndex=0;
    isRecording = 0;
    counterToggleLed=0;
    ledRec=0;
}

void seq_startRecord(void)
{
    seq_stopPlay();
    recordIndex=0;
    state =  SEQ_STATE_RECODING;
    isRecording = 1;
}

void seq_startPlay(void)
{
    state =  SEQ_STATE_PLAY;
    isRecording = 0;
}
void seq_stopPlay(void)
{
    if(state!=SEQ_STATE_IDLE)
    {
      midi_stopNote(0xFF);
      outs_set(OUT_REPEAT,0);      
      state = SEQ_STATE_IDLE;
    }  
}

void seq_startRecordNote(unsigned char noteNumber)
{
    if(recordIndex<SEQUENCE_LEN)
    {
      sequenceNoteDurationCounter=0;
      sequence[recordIndex].midiNoteNumber=noteNumber;
    }
  
}
void seq_endRecordNote(void)
{
    if(recordIndex<SEQUENCE_LEN)
    {
      sequence[recordIndex].duration = sequenceNoteDurationCounter;
      recordIndex++;
    }
}


void seq_stateMachine(void)
{
  switch(state)
  {
      case SEQ_STATE_IDLE:
      {
          break;
      }
      case SEQ_STATE_PLAY:
      {
        outs_set(OUT_MODE3,1);          
        playIndex=0;
        state = SEQ_STATE_PLAYING;
        break;
      }
      case SEQ_STATE_PLAYING:
      {
        outs_set(OUT_REPEAT,1);      
        midi_startNote(sequence[playIndex].midiNoteNumber);
        sequenceNoteDurationCounter=0;
        state = SEQ_STATE_WAIT_NOTE_DURATION;
        break;        
      }
      case SEQ_STATE_WAIT_NOTE_DURATION:
      {
        if(sequenceNoteDurationCounter>=sequence[playIndex].duration)
        {
            midi_stopNote(sequence[playIndex].midiNoteNumber);
            outs_set(OUT_REPEAT,0);      
            sequenceNoteDurationCounter=0;
            state = SEQ_STATE_WAIT_SILENCE_DURATION;
        }
        break;
      }
      case SEQ_STATE_WAIT_SILENCE_DURATION:
      {
        if(sequenceNoteDurationCounter>=silenceDuration)
        {
          playIndex++;
          if(playIndex>=recordIndex)
            playIndex=0;
          state = SEQ_STATE_PLAYING;          
        }
        break;
      }
      case SEQ_STATE_RECODING:
      {
        counterToggleLed++;
        if(counterToggleLed>350)
        {
          if(ledRec)
          {
            ledRec=0;
            outs_set(OUT_MODE3,0);          
          }
          else
          {
            ledRec=1;
            outs_set(OUT_MODE3,1);          
          }
        }
        break;
      }
  }
  
}

void seq_setSpeed(unsigned int val)
{
    silenceDuration = val ; 
}

unsigned char seq_isRecording(void)
{
  return isRecording;
}

