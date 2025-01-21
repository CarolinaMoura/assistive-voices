#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> 

// TFT color configuration
constexpr bool INVERT_COLORS = true;

// Debug enable
constexpr bool TO_DEBUG = true;

// SPI and serial communication settings
constexpr int CHIP_SELECT = 53;        // SPI chip select pin for SD card
constexpr uint32_t SERIAL_BAUDRATE = 250000; // Baud rate for Serial
constexpr uint32_t HARDWARE_BAUDRATE = 9600; // Baud rate for hardware communication
constexpr int RESISTANCE = 0 ; // little button resistance

// Display settings
constexpr uint16_t TFT_ID = 0x9486;   // Display ID for TFT
constexpr uint16_t SCREEN_WIDTH = 320;
constexpr uint16_t SCREEN_HEIGHT = 480;
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t BLACK = 0x0000;
constexpr uint16_t RED = 0xF800;
constexpr uint16_t LIGHT_GREEN = 0x2727;

// Button pins
constexpr int leftButtonPin = 8;
constexpr int rightButtonPin = 5;
constexpr int teacherButtonPin = 2;

// File and category settings
constexpr size_t MAX_SIZE_FILEARRAY = 100;
constexpr size_t MAX_SIZE_CATEGORIES = 100;

// Other settings
constexpr uint16_t thickness = 5;
constexpr int screenWords = 6;

// Extern declarations for global arrays
extern String categories[MAX_SIZE_CATEGORIES];
extern String fileArray[MAX_SIZE_FILEARRAY];

extern int categoriesCount, categoriesPtr, categoriesTempPtr, tempPtr;
extern int filesCount, filesPtr;
extern bool dialogue_mode , teacher_mode ;
extern String dialogue_sub ;

#endif // CONFIG_H
