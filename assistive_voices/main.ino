#include <SD.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <Fonts/FreeSans12pt7b.h> 
#include "DLabImage.h"
#define INVERT_COLORS true

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
int teacher_mode = false;

String* categories = nullptr;
int categoriesCount = 0, categoriesPtr = 0, categoriesTempPtr = 0, tempPtr = 0;

uint16_t adjustColor(uint16_t color) {
    return INVERT_COLORS ? ~color : color;
}
const uint16_t thickness = 5;
const int screenWords = 6;

String* fileArray = nullptr;
int filesCount = 0, filesPtr = 0;

// dimensions array for teacher_mode
int dimensions[6][4] = { 
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
};

// // words
// String words[] = {
//   "word1",
//   "word2",
//   "word3",
//   "word4",
//   "word5",
//   "word6",
//   "word7",
//   "word8",
//   "word9"
// };


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
  
  // categoriesCount = 9;
  // displayWords();
}

void displayWords() {
  // function that displays 6 categories at the time in teacher_mode

  Serial.println("Display words called");
  tft.fillScreen(adjustColor(WHITE)); // Clear the screen
  tft.setTextColor(adjustColor(0));
  tft.setFont(&FreeSans12pt7b);

  for (int i = 0; i < screenWords; i++) {
    // Extract the dimensions of the current word
    int16_t x1, y1;
    uint16_t w, h;
    int currentWord = (categoriesTempPtr + i) % categoriesCount;
    // Serial.println("adding word:" + String(currentWord));

    tft.setTextSize(2);
    tft.getTextBounds(categories[currentWord], 0, 0, &x1, &y1, &w, &h);
    if (w > tft.width() + 5) {
      tft.setTextSize(1);
      tft.getTextBounds(categories[currentWord], 0, 0, &x1, &y1, &w, &h);
    }

    // Set the cursor in the right position
    int x = (tft.width() - w) >> 1, y = 45 + (40+h)*i + (h >> 1);
    tft.setCursor(x, y);
    tft.print(categories[currentWord]);

    // save the dimensions
    dimensions[i][0] = x + x1 - 2*thickness, dimensions[i][1] = y + y1 - 2*thickness;
    dimensions[i][2] = w + 4*thickness, dimensions[i][3] = h + 4*thickness;
  }

  drawSelectSquare(adjustColor(RED), dimensions[0][0], dimensions[0][1], dimensions[0][2], dimensions[0][3], thickness);
}

void drawSelectSquare(uint16_t color, int x1, int y1, int w, int h, uint16_t thickness) {
  tft.fillRect(x1, y1, w, thickness, color);
  tft.fillRect(x1, y1 + h - thickness, w, thickness, color);
  tft.fillRect(x1, y1, thickness, h, color);
  tft.fillRect(x1 + w - thickness, y1, thickness, h, color);
}


void loop() {

  if (teacher_mode) {
    // teacher mode
    // Read button states
    rightButtonState = digitalRead(rightButtonPin);
    leftButtonState = digitalRead(leftButtonPin);
    
    if (rightButtonState == LOW) {
      if (tempPtr < screenWords-1) {
        tempPtr++;
        Serial.println("Next one: " + (String)(categoriesTempPtr + tempPtr));
        drawSelectSquare(adjustColor(WHITE), dimensions[tempPtr-1][0], dimensions[tempPtr-1][1], dimensions[tempPtr-1][2], dimensions[tempPtr-1][3], thickness);
        drawSelectSquare(adjustColor(RED), dimensions[tempPtr][0], dimensions[tempPtr][1], dimensions[tempPtr][2], dimensions[tempPtr][3], thickness);
        delay(500);
      } else {
        Serial.println("Renew screen.");
        tempPtr = 0;
        categoriesTempPtr = (categoriesTempPtr + screenWords) % categoriesCount;
        displayWords();
        delay(500);
      }
    } else if (leftButtonState == LOW) {
      categoriesPtr = (categoriesTempPtr + tempPtr) % categoriesCount;
      drawSelectSquare(adjustColor(LIGHT_GREEN), dimensions[tempPtr][0], dimensions[tempPtr][1], dimensions[tempPtr][2], dimensions[tempPtr][3], thickness);
      teacher_mode = false;
      delay(500);
    }

  } else {
    // student mode
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

void drawSquare(uint16_t color) {
  tft.fillRect(0, 0, SCREEN_WIDTH, 2*thickness, color);
  tft.fillRect(0, 0 + SCREEN_HEIGHT - 2*thickness, SCREEN_WIDTH, 2*thickness, color);
  tft.fillRect(0, 0, 2*thickness, SCREEN_HEIGHT, color);
  tft.fillRect(0 + SCREEN_WIDTH - 2*thickness, 0, 2*thickness, SCREEN_HEIGHT, color);
}

void displayImage(String filename) {
  DLabImage img(filename, SD);
  img.drawImage(tft, SD, false);
}
