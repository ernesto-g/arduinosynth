#define ANALOG_STATE_IDLE   0
#define ANALOG_STATE_START  1
#define ANALOG_STATE_WAIT   2
#define ANALOG_STATE_FINISH 3

void ain_init(void);
void ain_state_machine(void);
uint16_t* ain_getValues(void);
uint8_t ain_isReady(void);

