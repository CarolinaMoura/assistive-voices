#include "AudioHandler.h"
#include "Config.h"

void initializeAudio() {
  Serial3.begin( HARDWARE_BAUDRATE );
}

void adjustVolume(HardwareSerial &dfPlayerSerial, byte volume) {
  volume = constrain(volume, 0, 30);
  sendDFCommand(dfPlayerSerial, 0x06, volume);
}

void sendDFCommand(HardwareSerial &dfPlayerSerial, byte command, int param) {
  byte commandData[10];
  int checkSum = -(0xFF + 0x06 + command + 0x01 + highByte(param) + lowByte(param));

  commandData[0] = 0x7E;
  commandData[1] = 0xFF;
  commandData[2] = 0x06;
  commandData[3] = command;
  commandData[4] = 0x01; // Feedback enabled
  commandData[5] = highByte(param);
  commandData[6] = lowByte(param);
  commandData[7] = highByte(checkSum);
  commandData[8] = lowByte(checkSum);
  commandData[9] = 0xEF;

  for (int i = 0; i < 10; i++) {
    dfPlayerSerial.write(commandData[i]);
  }
}