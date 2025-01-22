#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <Arduino.h> 
#include "Config.h"
#include <SD.h>

String getCurrentDir() ;
void getContent(String dirname, String (*arr)[MAX_SIZE_CATEGORIES], int* count) ;
void listFiles(File dir, int numTabs) ;
String getCategoryName(String category) ;
String convertSpecialCharacters(String input) ;

#endif // FILE_UTILS_H