#define PIN_VCO1_SCALE  12
#define PIN_VCO2_SCALE  11
#define PIN_GATE_SIGNAL 8

#define MIDI_CURRENT_CHANNEL  0
#define MIDI_CMD_NOTE_ON  144
#define MIDI_CMD_NOTE_OFF  128

#define MIDI_STATE_IDLE       0
#define MIDI_STATE_RVC_DATA1  1
#define MIDI_STATE_RVC_DATA2  2
typedef struct S_MidiInfo {
  byte channel;
  byte cmd;
  byte note;
  byte vel;
}MidiInfo;

const unsigned char PROGMEM SINETABLE[10] = {0,1,2,3,4,5,6,7,8};
const unsigned char PROGMEM SAWTOOTHTABLE[10] = {0,1,2,3,4,5,6,7,8};
const unsigned short NOTES_TABLE_PWM[61] = {1003,990,975,960,944,927,909,891,871,849,827,803,778,753,723,693,661,627,592,554,514,471,426,379};

unsigned char toggle=0;
MidiInfo midiInfo;
byte midiStateMachine=MIDI_STATE_IDLE;
char keysActivatedCounter=0;


ISR(TIMER0_COMPA_vect)
{
  if(toggle==0)
  {
    //digitalWrite(LED_BUILTIN, HIGH);
    toggle=1;
  }
  else
  {
    //digitalWrite(LED_BUILTIN, LOW);
    toggle=0;
  }
}

void setup() {
  Serial.begin(31250); 

  // Set PB1/2 as outputs.
  DDRB |= (1 << DDB1) | (1 << DDB2);

  TCCR1A =
      (1 << COM1A1) | (1 << COM1B1) |
      // Fast PWM mode.
      (1 << WGM11);
  TCCR1B =
      // Fast PWM mode.
      (1 << WGM12) | (1 << WGM13) |
      // No clock prescaling (fastest possible
      // freq).
      (1 << CS10);
  OCR1A = 0;
  // Set the counter value that corresponds to
  // full duty cycle. For 15-bit PWM use
  // 0x7fff, etc. A lower value for ICR1 will
  // allow a faster PWM frequency.
  ICR1 = 1024;

  //**************** timer 0 
  /*
  // Set the Timer Mode to CTC
    TCCR2A = (1 << WGM21);
    // Set the value that you want to count to
    OCR2A = 0x80;
    // start the timer
    TCCR2B |= (1 << CS21) | (1 << CS20);        
  TIMSK2 |= _BV(OCIE2A);
  */

  cli();//stop interrupts

  // cada 133uS 
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 31;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  /*
  //set timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  */

sei();//allow interrupts

  //__________________________
  
  //pinMode(LED_BUILTIN,OUTPUT);  
  pinMode(PIN_VCO1_SCALE,OUTPUT);
  pinMode(PIN_VCO2_SCALE,OUTPUT);
  pinMode(PIN_GATE_SIGNAL,OUTPUT);
  digitalWrite(PIN_GATE_SIGNAL,LOW);
}



void midi_analizeMidiInfo(MidiInfo * pMidiInfo)
{
    
    if(pMidiInfo->channel==MIDI_CURRENT_CHANNEL)
    {
        if(pMidiInfo->cmd==MIDI_CMD_NOTE_ON)
        {
            // NOTE ON 
            //Serial.print("\r\nNOTE ON:");
            //Serial.print(pMidiInfo->note,DEC);
            //Serial.print(" vel:");            
            //Serial.print(pMidiInfo->vel,DEC);

            if(pMidiInfo->note>=36 && pMidiInfo->note<96)
            {
              unsigned short pwmVal = NOTES_TABLE_PWM[pMidiInfo->note-36];
              if(pMidiInfo->note<60)
              {
                digitalWrite(PIN_VCO1_SCALE, LOW);
                digitalWrite(PIN_VCO2_SCALE, LOW);
              }
              else
              {
                digitalWrite(PIN_VCO1_SCALE, HIGH);
                digitalWrite(PIN_VCO2_SCALE, HIGH);                
              }
              
              OCR1A = pwmVal;    
              OCR1B = pwmVal;  
              //Serial.print("PWM:");
              //Serial.print(pwmVal,DEC);   
              digitalWrite(PIN_GATE_SIGNAL,HIGH);
              keysActivatedCounter++;
            }
            //Serial.print("\r\n");    
        }
        else if(pMidiInfo->cmd==MIDI_CMD_NOTE_OFF)
        {
          // NOTE OFF
            //Serial.print("\r\nNOTE OF:");
            //Serial.print(pMidiInfo->note,DEC);
            //Serial.print(" vel:");            
            //Serial.print(pMidiInfo->vel,DEC);
            //Serial.print("\r\n");
            if(keysActivatedCounter>0)            
              keysActivatedCounter--;
            if(keysActivatedCounter==0)
              digitalWrite(PIN_GATE_SIGNAL,LOW);
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
  }
}
