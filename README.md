# Arduino Analog-Digital Synth

Analog Synth with MIDI input controlled by an arduino microcontroller. Some controls are connected to arduino and some are connected to the analog circuit.

Synth features:
  - MIDI input
  - 61 key support (36 to 96)
  - Two analog VCOs with:
    - Sine, sawtooth, triangle and square waveforms
    - tune control +- 4 notes 
    - +- 2 octave offset selector 
    - PWM 10% to 90% for square waveform
  - Two analog ADSRs with attack, decay, sustain and release controls
  - One digital 8 bit LFO with:
    - 0.5Hz to 30Hz tune control.
    - Sine, triangle, exponential and square waveforms
  - One analog filter with:
	- Three modes: High pass, Low pass and Band pass. 
	- Cutoff freq and resonance controls.
	- Modulated by LFO and ADSR2
  - One analog VCA modulated by ADSR1 and LFO
  - Analog pink and white noise generator
  - External audio input (line)  
  - Digital features:
      - LFO sync switch: Sync LFO beginning cicle with key hit.
      - Glissando switch: Play all notes from previous note to current note.
      - Arpeggiator: Repeat until 10 keys pressed at the same time with speed control.
      - Five modes for key priority: 
        - MODE 0:Monophonic. Last key pressed priority.
        - MODE 1:Monophonic. Lowest key pressed priority.
        - MODE 2:Monophonic. Highest key pressed priority.
        - MODE 3:Polyphonic (dual). Lowest and highest keys pressed priority.
        - MODE 4:(All leds off in panel) Sequencer mode. Record and play until 64 keys.

This project contains:
  - Arduino nano Firmware
  - Schematics
  - Panel templates
  - Pictures
  
  