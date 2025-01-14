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
  File openFile(SDClass &sd);
public:
  String path;
  char* caption;
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
  DLabImage(String path, SDClass &sd);

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
  void drawImage(MCUFRIEND_kbv tft, SDClass &sd, bool invertColors = true, uint16_t x = 0, uint16_t y = 0, uint16_t multFactor = 4);
};

#endif