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

  getContent("main", &categories, &category_count);
  getContent("main/conversa", &sub_dialogue, &dialogue_count);

  if (category_count == 0) return;

  displayCategories(&categories, category_count, category_tmp_idx);
}

void loop()
{
  if (teacher_mode)
  {
    // teacher mode to scroll through categories
    if (rightButton.stateChanged() && rightButton.read() == LOW)
    {
      scrollCategories(&categories, category_count, category_tmp_idx, category_screen_idx);
    }

    else if (leftButton.stateChanged() && leftButton.read() == LOW)
    {
      is_first_teacher_mode = false;
      selectCategory(&categories, category_count, category_idx, category_tmp_idx, category_screen_idx);
    }

  }
  
  else if (dialogue_mode)
  {
    // dialogue mode within student mode to build sentences by blocks
    if (rightButton.stateChanged() && rightButton.read() == LOW)
    {
      // scroll image
      getNextImageIn("main/" + categories[category_idx] + "/" + dialogue_sub);
    }
    if (leftButton.stateChanged() && leftButton.read() == LOW)
    {
      // select image
      selectImageIn("main/" + categories[category_idx] + "/" + dialogue_sub);
      // switch subfolders within the dialogue mode
      if (dialogue_sub == "bloque1")
      {
        dialogue_sub = fileArray[file_idx];
      }
      else
      {
        dialogue_sub = "bloque1";
      }
      getContent("main/" + categories[category_idx] + "/" + dialogue_sub, &fileArray, &file_count);
      file_idx = -1; // getNextImage will increase it by 1 when called
    }
  }
  else
  {
    // student mode to scroll through the options in a category
    if (rightButton.stateChanged() && rightButton.read() == LOW)
    {
      getNextImageIn("main/" + categories[category_idx]);
    }
    if (leftButton.stateChanged() && leftButton.read() == LOW)
    {
      selectImageIn("main/" + categories[category_idx]);
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