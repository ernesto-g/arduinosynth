#include <HardwareSerial.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "Config.h"
#include "AnalogIns.h"
#include "Lfo.h"
#include "MidiManager.h"
#include "FrontPanel.h"

static unsigned char state;
static unsigned char controlIndex;

static unsigned char getDiscrete4ValuesFromSwitchSelector(uint16_t analogValue);
static unsigned char getDiscrete5ValuesFromSwitchSelector(uint16_t analogValue);
static void setValueToManager(unsigned char controlIndex);

void frontp_init(void)
{
    state = FRONTPANEL_STATE_IDLE;
    
}

void frontp_state_machine(void)
{
    switch(state)
    {
        case FRONTPANEL_STATE_IDLE:
        {
          if(ain_isReady())
          {
            controlIndex=0;
            state = FRONTPANEL_STATE_SET_VALUE;
          }
          break;
        }
        case FRONTPANEL_STATE_SET_VALUE:
        {
          setValueToManager(controlIndex);
          controlIndex++;
          if(controlIndex>=8)
            state = FRONTPANEL_STATE_IDLE;
          break;
        }
    }
}

static void setValueToManager(unsigned char controlIndex)
{
    uint16_t* values = ain_getValues();
    switch(controlIndex)
    {
      case FRONTPANEL_ANALOG_INPUT_LFO_FREQ:
      {
        lfo_setFrequencyMultiplier(values[FRONTPANEL_ANALOG_INPUT_LFO_FREQ]);
        break;
      }
      case FRONTPANEL_ANALOG_INPUT_LFO_WAVEFORM:
      {
        unsigned char lfoWaveType = getDiscrete4ValuesFromSwitchSelector(values[FRONTPANEL_ANALOG_INPUT_LFO_WAVEFORM]);
        lfo_setWaveType(lfoWaveType);
        break;
      }
      case FRONTPANEL_ANALOG_INPUT_REPEAT_SPEED:
      {
        midi_setRepeatValue(values[FRONTPANEL_ANALOG_INPUT_REPEAT_SPEED]);
        break;
      }
      case FRONTPANEL_ANALOG_INPUT_LFO_SYNC_ON_OFF:
      {
        midi_setLfoSync(values[FRONTPANEL_ANALOG_INPUT_LFO_SYNC_ON_OFF]);
        break;
      }
      case FRONTPANEL_ANALOG_INPUT_VCO1_FINE_TUNE:
      {
        signed int tune = values[FRONTPANEL_ANALOG_INPUT_VCO1_FINE_TUNE] - 512;
        if(tune<16 && tune > -16)
          tune=0; // thick center value          
        midi_setTuneVco1(tune);
        break;
      }
      case FRONTPANEL_ANALOG_INPUT_VCO2_FINE_TUNE:
      {
        signed int tune = values[FRONTPANEL_ANALOG_INPUT_VCO2_FINE_TUNE] - 512;
        if(tune<16 && tune > -16)
          tune=0; // thick center value          
        midi_setTuneVco2(tune);
        break;
      }
      case FRONTPANEL_ANALOG_INPUT_VCO1_OCTAVE:
      {
        unsigned char octave = getDiscrete5ValuesFromSwitchSelector(values[FRONTPANEL_ANALOG_INPUT_VCO1_OCTAVE]);
        midi_setOctaveVco1(octave);
        break;
      }
      case FRONTPANEL_ANALOG_INPUT_VCO2_OCTAVE:
      {
        unsigned char octave = getDiscrete5ValuesFromSwitchSelector(values[FRONTPANEL_ANALOG_INPUT_VCO2_OCTAVE]);
        midi_setOctaveVco2(octave);
        break;
      }
    }
}

static unsigned char getDiscrete4ValuesFromSwitchSelector(uint16_t analogValue)
{
    if(analogValue>=0 && analogValue<256)
      return 0;
    if(analogValue>=256 && analogValue<512)
      return 1;
    if(analogValue>=512 && analogValue<768)
      return 2;
    if(analogValue>=768)
      return 3;      
}

static unsigned char getDiscrete5ValuesFromSwitchSelector(uint16_t analogValue)
{
    if(analogValue>=0 && analogValue<204)
      return 0;
    if(analogValue>=204 && analogValue<408)
      return 1;
    if(analogValue>=408 && analogValue<612)
      return 2;
    if(analogValue>=612 && analogValue<816)
      return 3;
    if(analogValue>=816)
      return 4;      
}


