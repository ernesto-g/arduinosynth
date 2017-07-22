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
#include "Config.h"
#include "MidiManager.h"
#include "Lfo.h"
#include "AnalogIns.h"
#include "FrontPanel.h"
#include "Outs.h"
#include "SequencerManager.h"

void setup() {
  config_init();
  outs_init();
  midi_init();
  lfo_init();
  ain_init();
  frontp_init();
  seq_init();
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

    frontp_state_machine();

    midi_repeatManager();
    midi_glissManager();

    outs_stateMachine();

    seq_stateMachine();
  }
}
