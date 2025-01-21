#include "FileUtils.h"
#include "MemoryHandler.h"


// arrays
String categories[MAX_SIZE_CATEGORIES];
String fileArray[MAX_SIZE_FILEARRAY];

// global variables
int categoriesCount = 0, categoriesPtr = 0, categoriesTempPtr = 0, tempPtr = 0;
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
    Serial.print("Memory before reading directory: ");
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
      Serial.print("Memory after reading file: ");
      Serial.println(getFreeMemory());
    }

    entry.close();
  }

  dir.close();

  if ( TO_DEBUG ) {
    Serial.print("Memory after reading directory: ");
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

String getCategoryName(String category) {
  File nameFile = SD.open("main/" + category + "/name.txt");
  if(!nameFile) return category;
  return nameFile.readStringUntil( '\n' ) ;
}