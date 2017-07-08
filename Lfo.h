void lfo_init(void);

#define LFO_WAVE_TYPE_SINE      3
#define LFO_WAVE_TYPE_TRIANGLE  2
#define LFO_WAVE_TYPE_EXP       1
#define LFO_WAVE_TYPE_SQUARE    0

void lfo_setWaveType(unsigned char type);
void lfo_setFrequencyMultiplier(unsigned int fm);


