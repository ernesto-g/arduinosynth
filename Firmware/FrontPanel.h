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
#define FRONTPANEL_ANALOG_INPUT_LFO_FREQ        5
#define FRONTPANEL_ANALOG_INPUT_LFO_WAVEFORM    2
#define FRONTPANEL_ANALOG_INPUT_REPEAT_SPEED    7
#define FRONTPANEL_ANALOG_INPUT_VCO1_FINE_TUNE  3
#define FRONTPANEL_ANALOG_INPUT_VCO2_FINE_TUNE  4
#define FRONTPANEL_ANALOG_INPUT_VCO1_OCTAVE     0
#define FRONTPANEL_ANALOG_INPUT_VCO2_OCTAVE     1

#define FRONTPANEL_ANALOG_INPUT_LFO_SYNC_ON_OFF     6


#define FRONTPANEL_STATE_IDLE         0
#define FRONTPANEL_STATE_SET_VALUE    1
#define FRONTPANEL_STATE_UPDATE_GLISS 2

#define FRONTPANEL_BTN_STATE_IDLE   0
#define FRONTPANEL_BTN_WAIT_RELEASE 1
#define FRONTPANEL_BTN_END_RELEASE  2



void frontp_init(void);
void frontp_state_machine(void);


