void lfo_init(void);

#define LFO_WAVE_TYPE_SINE      0
#define LFO_WAVE_TYPE_TRIANGLE  1
#define LFO_WAVE_TYPE_EXP       2
#define LFO_WAVE_TYPE_SQUARE    3

void lfo_setWaveType(unsigned char type);
void lfo_setFrequencyMultiplier(unsigned int fm);


