#ifndef CONFIG_H_
#define CONFIG_H_
const char *VERSION = "2.0beta";
const char *SETTINGS_FILE = "data/bpulse.cfg";
const float CEN_X = 64;
const float CEN_Y = 64;
const float R0 = 0.0;
const float R1 = R0 + 30.0;
const float R2 = R1 + 20.0;
const float R3 = R2 + 10.0;
const float R4 = R3 + 4.0;
const unsigned int FRAME_RATE = 25;
const unsigned int SYS = 0;
const unsigned int USER = 1;
const unsigned int NICE = 2;
const unsigned int IN = 0;
const unsigned int OUT = 1;
const unsigned int SENT = 0;
const unsigned int RECV = 1;
#endif // End of CONFIG_H_
