void lfo_init(void);

#define LFO_WAVE_TYPE_SINE      0
#define LFO_WAVE_TYPE_TRIANGLE  1

void lfo_setWaveType(unsigned char type);
void lfo_setFrequencyMultiplier(unsigned int fm);


