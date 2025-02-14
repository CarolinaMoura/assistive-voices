#ifndef DISPLAY_H
#define DISPLAY_H 

#include <Arduino.h> 
#include <MCUFRIEND_kbv.h>
#include "FileUtils.h"

void initializeDisplay();
void displayImage(const String& filename);
void displayCategories(String (*arr)[MAX_SIZE_CATEGORIES], int arr_size, int &tempPtr, int &screenPtr);
void scrollCategories(String (*arr)[MAX_SIZE_CATEGORIES], int arr_size, int &tempPtr, int &screenPtr);
void selectCategory(String (*arr)[MAX_SIZE_CATEGORIES], int arr_size, int &arrPtr, int &tempPtr, int &screenPtr, bool audio);
void drawSelectSquare(uint16_t color, int x1, int y1, int w, int h, uint16_t thickness);
void switchTeacherMode() ;
void drawSquare(uint16_t color);
void getNextImageIn(String folder_path);
void selectImageIn(String folder_path, bool audio);
uint16_t adjustColor(uint16_t color);

#endif // DISPLAY_H