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
void seq_init(void);
void seq_startRecord(void);
void seq_startPlay(void);
void seq_startRecordNote(unsigned char noteNumber);
void seq_endRecordNote(void);
void seq_stateMachine(void);
void seq_setSpeed(unsigned int val);
unsigned char seq_isRecording(void);
void seq_stopPlay(void);



