#include <SD.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <Fonts/FreeSans12pt7b.h> 
#include "DLabImage.h"
#include "Debounce.h"
#define INVERT_COLORS false

#define WHITE 0xFFFF
#define BLACK 0x0
#define RED 0xF800
#define LIGHT_GREEN 0x2727
#define RESISTANCE 0 // little resistance 

MCUFRIEND_kbv tft;

unsigned long lastDebounceTime = 0;  // Timestamp of the last button press
const unsigned long debounceDelay = 50; 

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 480;

const int leftButtonPin = 8;
const int rightButtonPin = 5;
const int teacherButtonPin = 2;

Debounce leftButton( leftButtonPin, RESISTANCE) ;
Debounce rightButton( rightButtonPin , RESISTANCE) ;
Debounce teacherButton( teacherButtonPin , RESISTANCE ) ;

int counter = 0;
bool teacher_mode = false;
bool dialogue_mode = false;
String dialogue_sub = "Bloque1";

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


void setup() {
  Serial.begin(250000);
  const int ID = 0x9486;
  tft.begin(ID);
  tft.fillScreen(adjustColor(TFT_WHITE));

  leftButton.begin();
  rightButton.begin();
  teacherButton.begin();

  if (!SD.begin(53)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  tft.setRotation(2);
  
  getContent("main", &categories, &categoriesCount);

  if(categoriesCount == 0) return;

  getContent("main/" + categories[0], &fileArray, &filesCount);
  if (categories[0] == "dialogo") {
    dialogue_mode = true;
    displayImage("main/dialogo/bloque1/" + fileArray[0]);
  } else {
    displayImage("main/" + categories[0] + "/" + fileArray[0]);
  }
}
 
void displayWords() {
  // function that displays 6 categories at the time in teacher_mode

  Serial.println("Display words called");
  tft.fillScreen(adjustColor(WHITE)); // Clear the screen
  tft.setTextColor(adjustColor(0));
  tft.setFont(&FreeSans12pt7b);
  
  // get standard word weight
  int16_t std_x=0, std_y=0, std_w=0, std_h = 0;
  tft.setTextSize(2);
  tft.getTextBounds("word", 0, 0, &std_x, &std_y, &std_w, &std_h);

  for (int i = 0; i < screenWords; i++) {
    // Extract the dimensions of the current word
    int16_t x1, y1;
    uint16_t w, h;
    int currentWord = (categoriesTempPtr + i) % categoriesCount;

    tft.setTextSize(2);
    tft.getTextBounds(categories[currentWord], 0, 0, &x1, &y1, &w, &h);
    if (w > tft.width() + 5) {
      tft.setTextSize(1);
      tft.getTextBounds(categories[currentWord], 0, 0, &x1, &y1, &w, &h);
    }

    // Set the cursor in the right position
    int x = (tft.width() - w) >> 1, y = 45 + (40+std_h)*i + (std_h >> 1);
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
    // teacher mode to scroll through categories

    if (rightButton.stateChanged() && rightButton.read() == LOW) {
      if (tempPtr < screenWords-1) {
        tempPtr++;
        drawSelectSquare(adjustColor(WHITE), dimensions[tempPtr-1][0], dimensions[tempPtr-1][1], dimensions[tempPtr-1][2], dimensions[tempPtr-1][3], thickness);
        drawSelectSquare(adjustColor(RED), dimensions[tempPtr][0], dimensions[tempPtr][1], dimensions[tempPtr][2], dimensions[tempPtr][3], thickness);
      } else {
        tempPtr = 0;
        categoriesTempPtr = (categoriesTempPtr + screenWords) % categoriesCount;
        displayWords();
      }

    } else if (leftButton.stateChanged() && leftButton.read() == LOW) {
      categoriesPtr = (categoriesTempPtr + tempPtr) % categoriesCount;
      drawSelectSquare(adjustColor(LIGHT_GREEN), dimensions[tempPtr][0], dimensions[tempPtr][1], dimensions[tempPtr][2], dimensions[tempPtr][3], thickness);
      teacher_mode = false;
      delay(500);

      // retrieve images from that folder and initiate student mode within that folder
      tft.fillScreen(adjustColor(WHITE)); // Clear the screen
      if (categories[categoriesPtr] == "dialogo") {
        dialogue_mode = true;
        getContent("main/Dialogo/Bloque1", &fileArray, &filesCount);
        Serial.println(fileArray[0]);
        displayImage("main/Dialogo/Bloque1/" + fileArray[0]);
        dialogue_sub = "Bloque1";
        Serial.println("dialogo, bloque1, " + fileArray[0]);
      } else {
        dialogue_mode = false;
        getContent("main/" + categories[categoriesPtr], &fileArray, &filesCount);
        displayImage("main/" + categories[categoriesPtr] + "/" + fileArray[0]);
        Serial.println(categories[categoriesPtr] + ", " + fileArray[0]);
      }

      // reset the temporary and image pointers
      categoriesTempPtr = categoriesPtr, tempPtr = 0, filesPtr = 0;
    }

  } else if (dialogue_mode) {
    // dialogue mode within student mode to build sentences by bloques

    if (rightButton.stateChanged() && rightButton.read() == LOW) {
      getNextImageIn("main/Dialogo/" + dialogue_sub);
    }

    if (leftButton.stateChanged() && leftButton.read() == LOW) {
      selectImageIn("main/Dialogo/" + dialogue_sub);
      if (dialogue_sub == "Bloque1") {
        dialogue_sub = fileArray[filesPtr];
      } else {
        dialogue_sub = "Bloque1";
      }
      getContent("main/Dialogo/" + dialogue_sub, &fileArray, &filesCount);
      filesPtr = 0;
      delay(500);
      displayImage("main/Dialogo/" + dialogue_sub + "/" + fileArray[0]);
    }

  } else {
    // student mode to scroll through the options in a category

    if (rightButton.stateChanged() && rightButton.read() == LOW) {
      getNextImageIn("main/" + categories[categoriesPtr]);
    }

    if (leftButton.stateChanged() && leftButton.read() == LOW) {
      selectImageIn("main/" + categories[categoriesPtr]);
    }
  }

  if (teacherButton.stateChanged() && teacherButton.read() == LOW) {
    tft.fillScreen(adjustColor(WHITE));
    teacher_mode = !(teacher_mode);
    if (teacher_mode) {
      categoriesTempPtr = categoriesPtr, tempPtr = 0;
      displayWords();
    } else {
      displayImage("main/" + categories[categoriesPtr] + "/" + fileArray[filesPtr]);
    }
  }
}

void getNextImageIn(String folder_path) {
  tft.fillScreen(adjustColor(WHITE));
  (++filesPtr) %= filesCount;
  displayImage(folder_path + "/" + fileArray[filesPtr]);
}

void selectImageIn(String folder_path) {
  uint16_t color = LIGHT_GREEN;
  drawSquare(color);
  String file_name = folder_path + "/" + fileArray[filesPtr] ; 
  DLabImage selected_img( file_name , SD ) ;
  int track = selected_img.getAudioFile( ) ;
  sendDFCommand( Serial3 , 0x03 , track ) ;
}

void getContent(String dirname, String** arr, int* count) {
  *count = 0;
  *arr = nullptr;
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
  img.drawImage(tft, SD, INVERT_COLORS);
}

void sendDFCommand(HardwareSerial &dfPlayerSerial, byte command, int param) {
    byte commandData[10];
    int checkSum;

    commandData[0] = 0x7E;
    commandData[1] = 0xFF;
    commandData[2] = 0x06;
    commandData[3] = command;
    commandData[4] = 0x01; // Feedback enabled
    commandData[5] = highByte(param);
    commandData[6] = lowByte(param);

    checkSum = -(commandData[1] + commandData[2] + commandData[3] + commandData[4] + commandData[5] + commandData[6]);
    commandData[7] = highByte(checkSum);
    commandData[8] = lowByte(checkSum);
    commandData[9] = 0xEF;

    for (int i = 0; i < 10; i++) {
        dfPlayerSerial.write(commandData[i]);
    }

    // Debug feedback from DFPlayer
    delay(100);
    while (dfPlayerSerial.available()) {
        Serial.print("DFPlayer Response: ");
        Serial.println(dfPlayerSerial.read(), HEX);
    }
}
