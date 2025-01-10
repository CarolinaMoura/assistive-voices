#ifndef DLABIMAGE_H
#define DLABIMAGE_H

#include <SD.h>
#include <MCUFRIEND_kbv.h>


/**
 * @class DLabImage
 * @brief Represents an image stored on an SD card and provides functionality to render it on a display.
 * 
 * This class handles the retrieval of image data from an SD card, extracting metadata (such as
 * dimensions and captions) and drawing the image onto a screen using the MCUFRIEND_kbv library.
 */
class DLabImage {
private:
  File openFile(SDClass &sd) {
    File file = SD.open(path, FILE_READ);
    if (!file) {
      Serial.println("Failed to open file in path " + path);
      return;
    }
    return file;
  }
public:
  String path, caption;
  uint16_t width, height;

  /**
    * @brief Constructs a DLabImage object by reading metadata from the specified file.
    * 
    * The constructor reads the first few bytes of the file to extract the image dimensions
    * and caption. If the file cannot be opened or the metadata cannot be read, it logs an
    * error to the Serial Monitor.
    * 
    * @param path The file path of the image.
    * @param sd The SD card object used to access the file system.
    */
  DLabImage(String path, SDClass &sd)
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
    width = (dimensions[0] << 8) | dimensions[1];
    height = (dimensions[2] << 8) | dimensions[3];

    // Read caption
    uint8_t captionSize = file.read();
    this->caption = "";
    for (uint8_t i = 0; i < captionSize; i++)
      this->caption += (char)file.read();

    file.close();
  }

  /**
    * @brief Draws the image on the screen given the location of the
    * top-left coordinate (defaults to the origin). It also displays the
    * image caption below the image.
    * 
    * The image should fit in the screen given
    * the top-left corner.
    *
    * @param tft The display object to print on.
    * @param sd The SD card object to read the file from.
    * @param invertColors Whether or not to invert the colors. Defaults to true.
    * @param x The x-coordinate where the image will be drawn. Defaults to 0.
    * @param y The y-coordinate where the image will be drawn. Defaults to 0.
    * @param multFactor The amount of rows to read at once in the buffer. Defaults to 4.
    *                   Be mindful that a large `multFactor` may cause memory overflow.
    */
  void drawImage(MCUFRIEND_kbv tft, SDClass &sd, bool invertColors = true, uint16_t x = 0, uint16_t y = 0, uint16_t multFactor = 4) {
    File file = this->openFile(sd);
    uint16_t offset = 5 + (this->caption).length();
    file.seek(offset);

    const int bufferSize = this->width * multFactor;
    uint8_t pixelBuffer[bufferSize << 1];

    for (uint16_t y = 0; y < this->height; y += multFactor) {
      file.read(pixelBuffer, bufferSize << 1);
      if (invertColors) {
        for (uint16_t i = 0; i < bufferSize; i++)
          pixelBuffer[i] = ~pixelBuffer[i];
      }

      tft.setAddrWindow(x, y, x + this->width, y + multFactor);
      tft.pushColors(pixelBuffer, bufferSize, true);
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
    tft.getTextBounds(caption, 0,0, &x1, &y1, &w, &h);
    if(w > tft.width() + 5) {
      tft.setTextSize(1);
      tft.getTextBounds(caption, 0,0, &x1, &y1, &w, &h);
    }

    // Set the cursor in the right position
    tft.setCursor((tft.width()-w)>>1, this->height+30+(h>>1));
    tft.print(caption);
  }
};

#endif