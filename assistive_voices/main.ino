#include <SD.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <Fonts/FreeSans12pt7b.h> 
#include "DLabImage.h"
#define INVERT_COLORS false

#define WHITE 0xFFFF
#define BLACK 0x0
#define RED 0xF800
#define LIGHT_GREEN 0x2727

MCUFRIEND_kbv tft;

unsigned long lastDebounceTime = 0;  // Timestamp of the last button press
const unsigned long debounceDelay = 50; 

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 480;

const int leftButtonPin = 2;     // blue
const int rightButtonPin = 3;    // red
const int leftLedPin = 9;
const int rightLedPin = 8;

int leftButtonState = 0;
int rightButtonState = 0;
int counter = 0;

String* categories = nullptr;
int categoriesCount = 0, categoriesPtr = 0;

uint16_t adjustColor(uint16_t color) {
    return INVERT_COLORS ? ~color : color;
}

String* fileArray = nullptr;
int filesCount = 0, filesPtr = 0;

void setup() {
  Serial.begin(250000);
  const int ID = 0x9486;
  tft.begin(ID);
  tft.fillScreen(adjustColor(TFT_WHITE));

  pinMode(leftButtonPin, INPUT);
  pinMode(rightButtonPin, INPUT);

  if (!SD.begin(53)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  tft.setRotation(2);
  
  getContent("main", &categories, &categoriesCount);

  if(categoriesCount == 0) return;

  getContent("main/" + categories[0], &fileArray, &filesCount);
  displayImage("main/" + categories[0] + "/" + fileArray[0]);
}

void getContent(String dirname, String** arr, int* count) {
  File dir = SD.open(dirname);
  if(!dir.isDirectory()){
    Serial.println("Trying to open something that is not a folder");
    return;
  }

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
        break;
    }

    String name = entry.name();

    if (name[0] == '_') {
        entry.close();
        continue;
    }

    String* tempArray = new String[*count + 1];
    for (int i = 0; i < *count; i++) {
        tempArray[i] = (*arr)[i];
    }
    delete[] (*arr);
    *arr = tempArray;

    String fileName = name; 
    for (int i = 0; i < fileName.length(); i++) {
        fileName[i] = tolower(fileName[i]); 
    }
    (*arr)[*count] = fileName;
    (*count)++;

    entry.close();
  }

  dir.close();
}

void listFiles(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    for (int i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());  // Print file name
    if (entry.isDirectory()) {
      Serial.println("/");  // Indicate it's a directory
      listFiles(entry, numTabs + 1);  // Recursively list files in the directory
    } else {
      // Files have a size
      Serial.print("\t");
      Serial.print(entry.size());
      Serial.println(" bytes");
    }
    entry.close();
  }
}

void loop() {
  // Read button states
  int leftRead = digitalRead(leftButtonPin);
  int lastState = leftButtonState;
  rightButtonState = digitalRead(rightButtonPin);

  leftButtonState = leftRead;

  if (lastState == HIGH && leftRead == LOW) {
    digitalWrite(leftLedPin, HIGH);
    tft.fillScreen(adjustColor(WHITE));
    (++filesPtr) %= filesCount;
    displayImage("main/" + categories[categoriesPtr] + "/" + fileArray[filesPtr]);
  }

  if (rightButtonState == LOW) {
    digitalWrite(rightLedPin, HIGH);
    uint16_t color = LIGHT_GREEN;
    drawSquare(color);
  } else {
    digitalWrite(rightLedPin, LOW);
  }
}

void drawSquare(uint16_t color) {
  uint16_t thickness = 10;
  tft.fillRect(0, 0, SCREEN_WIDTH, thickness, color);
  tft.fillRect(0, 0 + SCREEN_HEIGHT - thickness, SCREEN_WIDTH, thickness, color);
  tft.fillRect(0, 0, thickness, SCREEN_HEIGHT, color);
  tft.fillRect(0 + SCREEN_WIDTH - thickness, 0, thickness, SCREEN_HEIGHT, color);
}

void displayImage(String filename) {
  DLabImage img(filename, SD);
  img.drawImage(tft, SD, false);
}
