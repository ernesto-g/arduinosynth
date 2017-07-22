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
void lfo_init(void);

#define LFO_WAVE_TYPE_SINE      3
#define LFO_WAVE_TYPE_TRIANGLE  2
#define LFO_WAVE_TYPE_EXP       1
#define LFO_WAVE_TYPE_SQUARE    0

void lfo_setWaveType(unsigned char type);
void lfo_setFrequencyMultiplier(unsigned int fm);
void lfo_reset(void);


