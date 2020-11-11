#define HIGH 0x1
#define LOW 0x0

#define BEGIN 0x1
#define END 0x0

void init_timer(void);
void init_ext_int(void);
void sendCommand(int value);
int calc_cm(int counter);