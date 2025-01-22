#include <Adafruit_GFX.h>
#include "Display.h"
#include "DLabImage.h"
#include "AudioHandler.h"
#include "CustomFont12pt7b.h"

// Global Display Object
MCUFRIEND_kbv tft;

// dimensions array for teacher_mode
int dimensions[6][4] = { 
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
};

// global variables
bool teacher_mode = true;
bool dialogue_mode = false;
String dialogue_sub = "bloque1";

void initializeDisplay() {
  tft.begin( TFT_ID );
  tft.setRotation(0);
  tft.fillScreen(adjustColor( TFT_WHITE ));

  tft.setTextColor(adjustColor(TFT_BLACK)); // Black text with white background
  tft.setFont(&rroboto12pt8b);         // Use the custom font
  tft.setTextSize(2);
  tft.setCursor(50, 80);                  // Set the cursor position

}

void displayImage(const String& filename) {
  DLabImage img(filename, SD);
  img.drawImage(tft, SD, INVERT_COLORS);
}


void displayCategories() {
  // function that displays 6 categories at the time in teacher_mode

  if ( TO_DEBUG ) Serial.println("Display words called");
  tft.fillScreen( adjustColor( WHITE ) ); // Clear the screen
  tft.setTextColor( adjustColor( BLACK ) );
  // tft.setFont(&FreeSans12pt7b);
  
  // get font size and standard word height in font
  int16_t font_size = 0, std_x=0, std_y=0, std_w=0, std_h = 0;
  while (13*std_h < SCREEN_HEIGHT) {
    font_size++;
    tft.setTextSize(font_size);
    tft.getTextBounds("wordwordwo", 0, 0, &std_x, &std_y, &std_w, &std_h);
    if (std_w > SCREEN_WIDTH) {
      break;
    }
    Serial.println(String(font_size) + " " + String(std_h));
  }
  font_size--;
  tft.setTextSize(font_size);
  tft.getTextBounds("word", 0, 0, &std_x, &std_y, &std_w, &std_h);
  int std_spacing = (SCREEN_HEIGHT - 6*std_h) / 7;
  Serial.println(String(font_size) + " " + String(std_h) + " " + String(std_spacing));

  for (int i = 0; i < screenWords; i++) {
    // Extract the dimensions of the current word
    int16_t x1, y1;
    uint16_t w, h;
    int currentWord = (categoriesTempPtr + i) % categoriesCount;

    String category_name = getCategoryName(categories[currentWord]);

    tft.setTextSize(font_size);
    tft.getTextBounds(category_name, 0, 0, &x1, &y1, &w, &h);
    if (w > tft.width() + 5) {
      tft.setTextSize(font_size-1);
      tft.getTextBounds(category_name, 0, 0, &x1, &y1, &w, &h);
    }

    // Set the cursor in the right position
    int x = (tft.width() - w) >> 1;
    int y = std_spacing*(i+1) + std_h*i;
    tft.setCursor(x, y);
    tft.print(convertSpecialCharacters(category_name));
    
    // save the dimensions
    dimensions[i][0] = x + x1 - 2*thickness, dimensions[i][1] = y + y1 - 2*thickness;
    dimensions[i][2] = w + 4*thickness, dimensions[i][3] = h + 4*thickness;
  }
  
  drawSelectSquare(adjustColor(RED), dimensions[0][0], dimensions[0][1], dimensions[0][2], dimensions[0][3], thickness);
}

void scrollCategories() {
  // scrolling through categories during teacher mode
  if (tempPtr < screenWords-1) {
    // scroll downwards in screen
    tempPtr++;
    drawSelectSquare(adjustColor(WHITE), dimensions[tempPtr-1][0], dimensions[tempPtr-1][1], dimensions[tempPtr-1][2], dimensions[tempPtr-1][3], thickness);
    drawSelectSquare(adjustColor(RED), dimensions[tempPtr][0], dimensions[tempPtr][1], dimensions[tempPtr][2], dimensions[tempPtr][3], thickness);
  } else {
    // renew categories in screen
    tempPtr = 0;
    categoriesTempPtr = (categoriesTempPtr + screenWords) % categoriesCount;
    displayCategories();
  }
}

void selectCategory() {
  // selecting a category during teacher mode
  categoriesPtr = (categoriesTempPtr + tempPtr) % categoriesCount;
  drawSelectSquare(adjustColor(LIGHT_GREEN), dimensions[tempPtr][0], dimensions[tempPtr][1], dimensions[tempPtr][2], dimensions[tempPtr][3], thickness);
  teacher_mode = false;
  delay(500);

  // retrieve images from that folder and initiate student mode within that folder
  tft.fillScreen(adjustColor(WHITE)); // Clear the screen

  if (categories[categoriesPtr] == "conversa") {
    dialogue_mode = true;
    dialogue_sub = "bloque1";
    getContent("main/" + categories[categoriesPtr] + "/" + dialogue_sub, &fileArray, &filesCount);
    Serial.println(fileArray[0]);
    displayImage("main/" + categories[categoriesPtr] + "/" + dialogue_sub + "/" + fileArray[0]);

  } else {
    dialogue_mode = false;
    getContent("main/" + categories[categoriesPtr], &fileArray, &filesCount);
    displayImage("main/" + categories[categoriesPtr] + "/" + fileArray[0]);
    Serial.println(categories[categoriesPtr] + ", " + fileArray[0]);
  }

  // reset the temporary and image pointers
  categoriesTempPtr = categoriesPtr, tempPtr = 0, filesPtr = 0;
}

void drawSelectSquare(uint16_t color, int x1, int y1, int w, int h, uint16_t thickness) {
  tft.fillRect(x1, y1, w, thickness, color);
  tft.fillRect(x1, y1 + h - thickness, w, thickness, color);
  tft.fillRect(x1, y1, thickness, h, color);
  tft.fillRect(x1 + w - thickness, y1, thickness, h, color);
}


void switchTeacherMode() {
  // activate or deactivate teacher mode
  tft.fillScreen(adjustColor(WHITE));
  teacher_mode = !(teacher_mode);

  if (teacher_mode) {
    //start teacher mode
    categoriesTempPtr = categoriesPtr, tempPtr = 0;
    displayCategories();

  } else {
    // return to previous category and image, as no new one selected
    if (dialogue_mode) {
      displayImage("main/" + categories[categoriesPtr] + "/" + dialogue_sub + "/" + fileArray[filesPtr]);
    } else {
      displayImage("main/" + categories[categoriesPtr] + "/" + fileArray[filesPtr]);
    }
  }
}

void drawSquare(uint16_t color) {
  color = adjustColor(color);
  tft.fillRect(0, 0, SCREEN_WIDTH, 2*thickness, color);
  tft.fillRect(0, 0 + SCREEN_HEIGHT - 2*thickness, SCREEN_WIDTH, 2*thickness, color);
  tft.fillRect(0, 0, 2*thickness, SCREEN_HEIGHT, color);
  tft.fillRect(0 + SCREEN_WIDTH - 2*thickness, 0, 2*thickness, SCREEN_HEIGHT, color);
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

uint16_t adjustColor(uint16_t color) {
    return INVERT_COLORS ? ~color : color;
}