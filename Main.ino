#include "Config.h"
#include "MidiManager.h"
#include "Lfo.h"
#include "AnalogIns.h"

void setup() {
  config_init();
  midi_init();
  lfo_init();
  ain_init();
}


void loop() {
  Serial.print("INIT");
  while(1)
  {
    // MIDI Reception
    byte midiByte;
    if (Serial.available() > 0) 
    {
        midiByte = Serial.read();
        midi_stateMachine(midiByte);
    }
    //_______________    

    ain_state_machine();

  }
}
