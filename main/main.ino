#include <SD.h>
#include <MCUFRIEND_kbv.h>
#include "Adafruit_GFX.h"
#include "Debounce.h"
#include "Display.h"
#include "MemoryHandler.h"
#include "AudioHandler.h"

Debounce leftButton(leftButtonPin, RESISTANCE);
Debounce rightButton(rightButtonPin, RESISTANCE);
Debounce teacherButton(teacherButtonPin, RESISTANCE);

bool is_first_teacher_mode = true;

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
  getContent("main/conversa", &sub_dialogue, &dialogueCount);

  if (categoriesCount == 0) return;

  displayCategories(&categories, categoriesCount, categoriesTempPtr);
}

void loop()
{
  if (teacher_mode)
  {
    // teacher mode to scroll through categories
    if (rightButton.stateChanged() && rightButton.read() == LOW)
    {
      scrollCategories(&categories, categoriesCount, categoriesTempPtr, categoriesScreenPtr);
    }

    else if (leftButton.stateChanged() && leftButton.read() == LOW)
    {
<<<<<<< HEAD
<<<<<<< HEAD
      is_first_teacher_mode = false;
      selectCategory();
=======
      selectCategory(&categories, categoriesCount, &categoriesPtr, &categoriesTempPtr, &categoriesScreenPtr);
>>>>>>> a4fec0c (send indices as params)
=======
      selectCategory(&categories, categoriesCount, categoriesPtr, categoriesTempPtr, categoriesScreenPtr);
>>>>>>> 5b71bfb (send indices by reference and finish updating teacher_mode functions)
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
    if(is_first_teacher_mode) {
      teacher_mode = false;
    }
    switchTeacherMode();
  }
}