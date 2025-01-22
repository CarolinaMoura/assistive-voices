#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <Arduino.h>

// DFPlayer Mini functions
void initializeAudio();
void adjustVolume(HardwareSerial &dfPlayerSerial, byte volume);
void sendDFCommand(HardwareSerial &dfPlayerSerial, byte command, int param);

#endif