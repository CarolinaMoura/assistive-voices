#include <Adafruit_GFX.h>
#include "Display.h"
#include "DLabImage.h"
#include "AudioHandler.h"

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
const int font_size = 2;

void initializeDisplay() {
  tft.begin( TFT_ID );
  tft.setRotation(0);
  tft.fillScreen(adjustColor( TFT_WHITE ));

  tft.setTextColor(adjustColor(TFT_BLACK)); // Black text with white background
  tft.setFont(&ACTIVE_FONT);         // Use the custom font
  tft.setTextSize(font_size);
  tft.setCursor(50, 80);                  // Set the cursor position

}

void displayImage(const String& filename) {
  DLabImage img(filename, SD);
  img.drawImage(tft, SD, INVERT_COLORS);
}


void displayCategories() {
  // function that displays 6 categories at the time in teacher_mode
  int numWords = min(screenWords, categoriesCount);

  if ( TO_DEBUG ) Serial.println("Display words called");
  tft.fillScreen( adjustColor( WHITE ) ); // Clear the screen
  tft.setTextColor( adjustColor( BLACK ) );
  
  // get font size and standard word height in font
  int16_t std_x=0, std_y=0, std_w=0, std_h = 0;
  tft.setTextSize(font_size);
  tft.getTextBounds("word", 0, 0, &std_x, &std_y, &std_w, &std_h);
  int std_spacing = (SCREEN_HEIGHT - (numWords)*std_h) / (numWords+1);

  int x = 0, y = 0;
  for (int i = 0; i < numWords; i++) {
    // Extract the dimensions of the current word
    int16_t x1, y1;
    uint16_t w, h;
    int currentWord = (categoriesTempPtr + i) % categoriesCount;

    String category_name = getCategoryName(categories[currentWord]);

    tft.setTextSize(font_size);
    tft.getTextBounds(category_name, 0, 0, &x1, &y1, &w, &h);
    if (w > tft.width() - 5) {
      tft.setTextSize(font_size-1);
      Serial.println(font_size-1);
      tft.getTextBounds(category_name, 0, 0, &x1, &y1, &w, &h);
    }

    // Set the cursor in the right position
    x = (tft.width() - w) >> 1;
    y += std_spacing + std_h;
    tft.setCursor(x, y);
    tft.print(convertSpecialCharacters(category_name));
    
    // save the dimensions
    dimensions[i][0] = x + x1 - 2*thickness, dimensions[i][1] = y + y1 - 2*thickness;
    dimensions[i][2] = w + 4*thickness, dimensions[i][3] = h + 4*thickness;
  }
  
  drawSelectSquare(adjustColor(RED), dimensions[0][0], dimensions[0][1], dimensions[0][2], dimensions[0][3], thickness);
}

void scrollCategories(int arr_size, int *tempPtr, int *screenPtr) {
  int numWords = min(screenWords, arr_size);
  // scrolling through categories during teacher mode
  if ((*screenPtr) < numWords-1) {
    // scroll downwards in screen
    (*screenPtr)++;
    drawSelectSquare(adjustColor(WHITE), dimensions[(*screenPtr)-1][0], dimensions[(*screenPtr)-1][1], dimensions[(*screenPtr)-1][2], dimensions[(*screenPtr)-1][3], thickness);
    drawSelectSquare(adjustColor(RED), dimensions[(*screenPtr)][0], dimensions[(*screenPtr)][1], dimensions[(*screenPtr)][2], dimensions[(*screenPtr)][3], thickness);
  } else {
    // renew categories in screen
    *screenPtr = 0;
    *tempPtr = (*tempPtr + numWords) % arr_size;
    displayCategories();
  }
}

void selectCategory(String (*arr)[MAX_SIZE_CATEGORIES], int arr_size, int *arrPtr, int *tempPtr, int *screenPtr) {
  // selecting a category during teacher mode
  *arrPtr = (*tempPtr + *screenPtr) % arr_size;
  Serial.println(String(*arrPtr) + ", " + (*arr)[*arrPtr]);
  drawSelectSquare(adjustColor(LIGHT_GREEN), dimensions[*screenPtr][0], dimensions[*screenPtr][1], dimensions[*screenPtr][2], dimensions[*screenPtr][3], thickness);
  teacher_mode = false;
  delay(500);

  // retrieve images from that folder and initiate student mode within that folder
  tft.fillScreen(adjustColor(WHITE)); // Clear the screen

  if ((*arr)[*arrPtr] == "conversa") {
    dialogue_mode = true;
    // dialogue_sub = "bloque1";
    // getContent("main/" + categories[categoriesPtr] + "/" + dialogue_sub, &fileArray, &filesCount);
    // Serial.println(fileArray[0]);
    // displayImage("main/" + categories[categoriesPtr] + "/" + dialogue_sub + "/" + fileArray[0]);

  } else {
    dialogue_mode = false;
    getContent("main/" + (*arr)[*arrPtr], &fileArray, &filesCount);
    displayImage("main/" + (*arr)[*arrPtr] + "/" + fileArray[0]);
    Serial.println((*arr)[*arrPtr] + ", " + fileArray[0]);
  }

  // reset the temporary and image pointers
  *tempPtr = *arrPtr, *screenPtr = 0, filesPtr = 0;
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
    categoriesTempPtr = categoriesPtr, categoriesScreenPtr = 0;
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