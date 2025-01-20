#ifndef CONFIG_H
#define CONFIG_H

#define INVERT_COLORS false

#define CHIP_SELECT 53 
#define SERIAL_BAUDRATE 250000
#define HARDWARE_BAUDRATE 9600

#define TFT_ID 0x9486 
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT  480

#define MAX_SIZE_FILEARRAY 100
#define MAX_SIZE_CATEGORIES 100
#define WHITE 0xFFFF
#define BLACK 0x0
#define RED 0xF800
#define LIGHT_GREEN 0x2727
#define RESISTANCE 0 // little resistance 

const int leftButtonPin = 8;
const int rightButtonPin = 5;
const int teacherButtonPin = 2;

#endif