

void seq_init(void);
void seq_startRecord(void);
void seq_startPlay(void);
void seq_startRecordNote(unsigned char noteNumber);
void seq_endRecordNote(void);
void seq_stateMachine(void);
void seq_setSpeed(unsigned int val);
unsigned char seq_isRecording(void);
void seq_stopPlay(void);



