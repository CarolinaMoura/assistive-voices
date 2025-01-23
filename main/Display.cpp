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
bool dialogue_first_word = true;
const int font_size = 2;

void initializeDisplay() {
  tft.begin( TFT_ID );
  tft.setRotation(2);
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


void displayCategories(String (*arr)[MAX_SIZE_CATEGORIES], int arr_size, int &tempPtr, int &screenPtr) {
  // function that displays up to 'screenWords' categories at the time in teacher_mode
  int numWords = min(screenWords, arr_size);

  if ( TO_DEBUG ) Serial.println(F("Display words called"));
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
    int currentWord = (tempPtr + i) % arr_size;

    String category_name = getCategoryName((*arr)[currentWord]);

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
  
  drawSelectSquare(adjustColor(RED), dimensions[screenPtr][0], dimensions[screenPtr][1], dimensions[screenPtr][2], dimensions[screenPtr][3], thickness);
}

void scrollCategories(String (*arr)[MAX_SIZE_CATEGORIES], int arr_size, int &tempPtr, int &screenPtr) {
  int numWords = min(screenWords, arr_size);
  // scrolling through categories during teacher mode
  if ((screenPtr) < numWords-1) {
    // scroll downwards in screen
    (screenPtr)++;
    drawSelectSquare(adjustColor(WHITE), dimensions[(screenPtr)-1][0], dimensions[(screenPtr)-1][1], dimensions[(screenPtr)-1][2], dimensions[(screenPtr)-1][3], thickness);
    drawSelectSquare(adjustColor(RED), dimensions[(screenPtr)][0], dimensions[(screenPtr)][1], dimensions[(screenPtr)][2], dimensions[(screenPtr)][3], thickness);
  } else {
    // renew categories in screen
    screenPtr = 0;
    tempPtr = (tempPtr + numWords) % arr_size;
    displayCategories(arr, arr_size, tempPtr, screenPtr);
  }
}

void selectCategory(String (*arr)[MAX_SIZE_CATEGORIES], int arr_size, int &arrPtr, int &tempPtr, int &screenPtr, bool audio) {
  // selecting a category during current mode
  arrPtr = (tempPtr + screenPtr) % arr_size;
  if ( TO_DEBUG ) Serial.println(String(arrPtr) + ", " + (*arr)[arrPtr]);
  drawSelectSquare(adjustColor(LIGHT_GREEN), dimensions[screenPtr][0], dimensions[screenPtr][1], dimensions[screenPtr][2], dimensions[screenPtr][3], thickness);
  if (audio) {
    // play audio if possible
    // add a delay of 1300
  }
  delay(500);

  // retrieve images from that folder and initiate student mode within that folder
  tft.fillScreen(adjustColor(WHITE)); // Clear the screen

  if (teacher_mode) {
    teacher_mode = false;
    if ((*arr)[arrPtr] == "conversa") {
      dialogue_mode = true, dialogue_first_word = true;
      dialogue_idx = 0, dialogue_tmp_idx = 0; dialogue_screen_idx = 0;
      displayCategories(&dialogue_first_words, dialogue_count, dialogue_tmp_idx, dialogue_screen_idx);
    } else {
      dialogue_mode = false;
      getContent("main/" + (*arr)[arrPtr], &fileArray, &file_count);
      displayImage("main/" + (*arr)[arrPtr] + "/" + fileArray[0]);
    }
  } else if (dialogue_mode) {
    dialogue_first_word = false;
    getContent("main/" + categories[category_idx] + "/" + (*arr)[arrPtr], &fileArray, &file_count);
    displayImage("main/" + categories[category_idx] + "/" + (*arr)[arrPtr] + "/" + fileArray[0]);
  }

  // reset the temporary and image pointers
  tempPtr = arrPtr, screenPtr = 0, file_idx = 0;
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
    category_tmp_idx = category_idx, category_screen_idx = 0;
    displayCategories(&categories, category_count, category_tmp_idx, category_screen_idx);

  } else {
    // return to previous category and image, as no new one selected
    if (dialogue_mode) {
      if (dialogue_first_word) {
        dialogue_idx = 0, dialogue_tmp_idx = 0; dialogue_screen_idx = 0;
        displayCategories(&dialogue_first_words, dialogue_count, dialogue_tmp_idx, dialogue_screen_idx);
      } else {
        displayImage("main/" + categories[category_idx] + "/" + dialogue_first_words[dialogue_idx] + "/" + fileArray[file_idx]);
      }
    } else {
      displayImage("main/" + categories[category_idx] + "/" + fileArray[file_idx]);
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
  (++file_idx) %= file_count;
  displayImage(folder_path + "/" + fileArray[file_idx]);
}

void selectImageIn(String folder_path, bool audio) {
  uint16_t color = LIGHT_GREEN;
  drawSquare(color);
  String file_name = folder_path + "/" + fileArray[file_idx] ; 
  DLabImage selected_img( file_name , SD ) ;
  if (audio) {
    int track = selected_img.getAudioFile( ) ;
    sendDFCommand( Serial3 , 0x03 , track ) ;
  }
}

uint16_t adjustColor(uint16_t color) {
    return INVERT_COLORS ? ~color : color;
}