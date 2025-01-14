#include "DLabImage.h"
#include <Arduino.h>
#include <Fonts/FreeSans12pt7b.h> 


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

    Serial.println(this->caption);
    file.close();
  }

void DLabImage::drawImage(MCUFRIEND_kbv tft, SDClass &sd, bool invertColors = true, uint16_t x = 0, uint16_t y = 0, uint16_t multFactor = 4) {
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
          for (uint16_t i = 0; i < bufferSize; i++)
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

          Serial.println("Quantity: " + (String)qtt);
          Serial.println((String)h);
          Serial.println((String)l);

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

    tft.setTextSize(2);
    tft.setTextColor(invertColors ? ~0 : 0);
    tft.setFont(&FreeSans12pt7b);

    // Extract the dimensions of the caption
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(caption, 0, 0, &x1, &y1, &w, &h);
    if (w > tft.width() + 5) {
      tft.setTextSize(1);
      tft.getTextBounds(caption, 0, 0, &x1, &y1, &w, &h);
    }

    // Set the cursor in the right position
    tft.setCursor((tft.width() - w) >> 1, this->height + 30 + (h >> 1));
    tft.print(caption);
  }