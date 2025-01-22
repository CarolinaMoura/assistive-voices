#ifndef DISPLAY_H
#define DISPLAY_H 

#include <Arduino.h> 
#include <MCUFRIEND_kbv.h>
#include "FileUtils.h"

void initializeDisplay();
void displayImage(const String& filename);
void displayCategories();
void scrollCategories();
void selectCategory();
void drawSelectSquare(uint16_t color, int x1, int y1, int w, int h, uint16_t thickness);
void switchTeacherMode() ;
void drawSquare(uint16_t color);
void getNextImageIn(String folder_path);
void selectImageIn(String folder_path);
uint16_t adjustColor(uint16_t color);

#endif // DISPLAY_H