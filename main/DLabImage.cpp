#include <Arduino.h>
#include "DLabImage.h"
#include "FileUtils.h"

int DLabImage::levenshtein(String s1, String s2){
    int len1 = s1.length();
    int len2 = s2.length();
    int matrix[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            matrix[i][j] = fmin(
(matrix[i - 1][j] + 1, matrix[i][j - 1] + 1),
                matrix[i - 1][j - 1] + cost
            );
        }
    }

    return matrix[len1][len2];
}

File DLabImage::openFile(SDClass& sd) {
  File file = SD.open(this->path, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file in path " + path);
    return;
  }
  return file;
}

DLabImage::DLabImage(String path, SDClass &sd)
    : path(path), width(0), height(0), caption("") {
    File file = this->openFile(sd);

    // Extract the 4 bytes representing the width and height
    uint8_t dimensions[4];
    if (file.read(dimensions, 4) != 4) {
      Serial.println("Failed to read dimensions from file: " + path);
      file.close();
      return;
    }

    // Recalculate using the higher and lower bytes
    this->width = (dimensions[0] << 8) | dimensions[1];
    this->height = (dimensions[2] << 8) | dimensions[3];

    // Read caption
    char c = file.read();
    while(c != '\0') {
      this->caption += c;
      c = file.read();
    }

    file.close();
  }

void DLabImage::drawImage(MCUFRIEND_kbv tft, SDClass &sd, bool invertColors = true, uint16_t x = 0, uint16_t y = 0, uint16_t multFactor = 2) {
    File file = this->openFile(sd);
    uint16_t offset = 5 + (this->caption).length();
    file.seek(offset);

    uint8_t encodingType = file.read();

    const int bufferSize = this->width * multFactor;
    uint8_t pixelBuffer[bufferSize << 1];

    if(encodingType == 0){
      for (uint16_t y = 0; y < this->height; y += multFactor) {
        file.read(pixelBuffer, bufferSize << 1);
        if (invertColors) {
          for (uint16_t i = 0; i < (bufferSize << 1); i++)
            pixelBuffer[i] = ~pixelBuffer[i];
        }

        tft.setAddrWindow(x, y, x + this->width, y + multFactor);
        tft.pushColors(pixelBuffer, bufferSize, true);
      }
    } 
    else {
      const int bufferSize = this->width * multFactor;
      uint16_t rowBuffer[bufferSize];
      uint16_t currentX = 0;
      uint16_t currentY = 0;

      while (currentY < this->height) {
          uint8_t qtt = file.read();  
          uint8_t h = file.read();  
          uint8_t l = file.read();  

          uint16_t color = (h << 8) | l;

          if (invertColors) {
              color = ~color;
          }
          for (uint8_t i = 0; i < qtt; i++) {
              rowBuffer[currentX++] = color;

              if (currentX >= this->width) {
                  tft.setAddrWindow(x, currentY, x + this->width - 1, currentY);
                  tft.pushColors(rowBuffer, this->width, true);

                  currentX = 0;
                  currentY++;
                  if (currentY >= this->height) {
                      break;
                  }
              }
          }
      }
    }
    file.close();

    // The next part is gross...
    // We should think of a cleaner way to
    // implement this.
    int16_t font_size = 2;
    tft.setTextSize(font_size);
    tft.setTextColor(invertColors ? ~0 : 0);
    // tft.setFont(&FreeSans12pt7b);
    // tft.cp437(true);

    // Extract the dimensions of the caption
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(caption, 0, 0, &x1, &y1, &w, &h);
    if (w > tft.width() - 5) {
      tft.setTextSize(font_size - 1);
      tft.getTextBounds(caption, 0, 0, &x1, &y1, &w, &h);
    }

    // Set the cursor in the right position
    tft.setCursor((tft.width() - w) >> 1, this->height + 30 + (h >> 1));
    tft.println(convertSpecialCharacters(caption));
  }

int DLabImage::getAudioFile() {

    File metadata_file = SD.open( "metadata.txt" ) ;
    if ( !metadata_file ) {
        Serial.println("Unable to open metadata.txt") ;
        return ;
    }

    int track_number = 1, best = 1;
    int cur_track_number_score = (this->caption).length();
    String line ;

    while ( metadata_file.available() ) {
        line = metadata_file.readStringUntil( '\n' ) ;
        int score = this->levenshtein(line, this->caption);
        if (score < cur_track_number_score) {
          
          if(score == 0){
            metadata_file.close();
            return track_number ;
          }

          cur_track_number_score = score;
          best = track_number;
        }

        ++track_number ;
    }

    Serial.println(" No matching caption found in metadata.txt for '" + this->caption + "'");
    metadata_file.close();
    return best;
}