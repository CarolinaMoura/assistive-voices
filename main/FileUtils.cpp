#include "FileUtils.h"
#include "MemoryHandler.h"

// arrays
String categories[MAX_SIZE_CATEGORIES];
String fileArray[MAX_SIZE_FILEARRAY];
String sub_dialogue[MAX_SIZE_CATEGORIES];

// global variables
int categoriesCount = 0, categoriesPtr = 0, categoriesTempPtr = 0, categoriesScreenPtr = 0;
int dialogueCount = 0, dialoguePtr = 0, dialogueTempPtr = 0, dialogueScreenPtr = 0;
int filesCount = 0, filesPtr = 0;

String getCurrentDir() {
  return "main/" + categories[categoriesPtr] + "/" + fileArray[filesPtr];
}

void getContent(String dirname, String (*arr)[MAX_SIZE_CATEGORIES], int* count) {
  *count = 0;
  File dir = SD.open(dirname);
  if(!dir.isDirectory()){
    Serial.println("Trying to open something that is not a folder: " + dirname);
    return;
  }


  if ( TO_DEBUG ) {
    Serial.print(F("Memory before reading directory: "));
    Serial.println(getFreeMemory());
  }
  
  while (true) {

    if  ( *count >= MAX_SIZE_FILEARRAY ) {
      Serial.println("Too many files in directory. File count exceeds maximum of " + String( MAX_SIZE_FILEARRAY ) );
      break;  
    }

    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }

    String name = entry.name();

    if (name[0] == '_' || name == "NAME.TXT") {
      entry.close();
      continue;
    }

    String fileName = name; 
    
    for (int i = 0; i < fileName.length(); i++) {
      fileName[i] = tolower(fileName[i]); 
    }
    
    (*arr)[*count] = fileName;
    (*count)++;

    if ( TO_DEBUG ) {
      Serial.print(F("Memory after reading file: "));
      Serial.println(getFreeMemory());
    }

    entry.close();
  }

  dir.close();

  if ( TO_DEBUG ) {
    Serial.print(F("Memory after reading directory: "));
    Serial.println(getFreeMemory());
  }
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
    
    Serial.print(entry.name()); // Print file name
    if (entry.isDirectory()) {
      Serial.println(F("/"));  // Indicate it's a directory
      listFiles(entry, numTabs + 1);  // Recursively list files in the directory
    } else {
      // Files have a size
      Serial.print(F("\t"));
      Serial.print(entry.size());
      Serial.println(F(" bytes"));
    }
    entry.close();  
  }
}

String getCategoryName(String category) {
  File nameFile = SD.open("main/" + category + "/name.txt");
  if(!nameFile) return category;
  return convertSpecialCharacters(nameFile.readStringUntil( '\n' ) );
}

String convertSpecialCharacters(String input) {
  String output = "";

  for (int i = 0; i < input.length(); i++) {
    unsigned char c = input[i];  // Treat the character as unsigned

<<<<<<< HEAD
    if ( TO_DEBUG ) {
      //Serial.print(F("c: "));
      //Serial.println(c);
    }
=======
    // if ( TO_DEBUG ) {
    //   Serial.print("c: ");
    //   Serial.println(c);
    // }
>>>>>>> 5b71bfb (send indices by reference and finish updating teacher_mode functions)

    switch (c) {
      // map from extended ascii to ascii encoding for Spanish special characters.
      case 0xa0: output += "\xe1"; break;  // á -> 0xE1
      case 0x82: output += "\xe9"; break;  // é -> 0xE9
      case 0xa1: output += "\xed"; break;  // í -> 0xED
      case 0xa2: output += "\xf3"; break;  // ó -> 0xF3
      case 0xa3: output += "\xfa"; break;  // ú -> 0xFA
      case 0xa4: output += "\xf1"; break;  // ñ -> 0xF1
      case 0x81: output += "\xfc"; break;  // ü -> 0xFC
      case 0xa5: output += "\xbf"; break;  // ¿ -> 0xBF
      case 0xa6: output += "\xa1"; break;  // ¡ -> 0xA1
      default: output += input[i];  // Keep other characters as is
    }
  }

  return output;
}