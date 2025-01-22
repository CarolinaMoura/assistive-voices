#include <SD.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include "DLabImage.h"
#include "Debounce.h"
#include "AudioHandler.h"
#include "MemoryHandler.h"
#include "Display.h"
Debounce leftButton(leftButtonPin, RESISTANCE);
Debounce rightButton(rightButtonPin, RESISTANCE);
Debounce teacherButton(teacherButtonPin, RESISTANCE);

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);

  initializeAudio();
  initializeDisplay();

  leftButton.begin();
  rightButton.begin();
  teacherButton.begin();

  initializeSD();

  getContent("main", &categories, &categoriesCount);

  if (categoriesCount == 0)
    return;

  displayCategories();

  // getContent("main/" + categories[0], &fileArray, &filesCount);

  // if (categories[0] == "conversa") {
  //   dialogue_mode = true;
  //   displayImage("main/conversa/" + dialogue_sub + "/" + fileArray[0]);
  // } else {
  //   displayImage(getCurrentDir());
  // }

  // listFiles(SD.open("/main/"), 0);
}

void loop()
{
  if (teacher_mode)
  {
    // teacher mode to scroll through categories
    if (rightButton.stateChanged() && rightButton.read() == LOW)
    {
      scrollCategories();
    }
    else if (leftButton.stateChanged() && leftButton.read() == LOW)
    {
      selectCategory();
    }
  }
  else if (dialogue_mode)
  {
    // dialogue mode within student mode to build sentences by blocks
    if (rightButton.stateChanged() && rightButton.read() == LOW)
    {
      // scroll image
      getNextImageIn("main/" + categories[categoriesPtr] + "/" + dialogue_sub);
    }
    if (leftButton.stateChanged() && leftButton.read() == LOW)
    {
      // select image
      selectImageIn("main/" + categories[categoriesPtr] + "/" + dialogue_sub);
      // switch subfolders within the dialogue mode
      if (dialogue_sub == "bloque1")
      {
        dialogue_sub = fileArray[filesPtr];
      }
      else
      {
        dialogue_sub = "bloque1";
      }
      getContent("main/" + categories[categoriesPtr] + "/" + dialogue_sub, &fileArray, &filesCount);
      filesPtr = -1; // getNextImage will increase it by 1 when called
    }
  }
  else
  {
    // student mode to scroll through the options in a category
    if (rightButton.stateChanged() && rightButton.read() == LOW)
    {
      getNextImageIn("main/" + categories[categoriesPtr]);
    }
    if (leftButton.stateChanged() && leftButton.read() == LOW)
    {
      selectImageIn("main/" + categories[categoriesPtr]);
    }
  }

  if (teacherButton.stateChanged() && teacherButton.read() == LOW)
  {
    // activate or deactivate teacher mode
    switchTeacherMode();
  }
}