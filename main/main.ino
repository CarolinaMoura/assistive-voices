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
  getContent("main/conversa", &dialogue_first_words, &dialogue_count);

  if (category_count == 0) return;

  displayCategories(&categories, category_count, category_tmp_idx, category_screen_idx);
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
      selectCategory(&categories, category_count, category_idx, category_tmp_idx, category_screen_idx, false);
    }

  }
  
  else if (dialogue_mode)
  {
    // dialogue mode within student mode to build sentences by blocks
    if (dialogue_first_word) {
      // scroll amongst first word options
      if (rightButton.stateChanged() && rightButton.read() == LOW) {
        scrollCategories(&dialogue_first_words, dialogue_count, dialogue_tmp_idx, dialogue_screen_idx);
      } else if (leftButton.stateChanged() && leftButton.read() == LOW) {
        if (dialogue_first_words[dialogue_tmp_idx] == "dialogo") {
          selectCategory(&dialogue_first_words, dialogue_count, dialogue_idx, dialogue_tmp_idx, dialogue_screen_idx, false);
        } else {
          selectCategory(&dialogue_first_words, dialogue_count, dialogue_idx, dialogue_tmp_idx, dialogue_screen_idx, true);
        }
      }
    } else {
      //scroll within a category of second word options related to the first choice
      if (rightButton.stateChanged() && rightButton.read() == LOW) {
        // scroll image
        getNextImageIn("main/" + categories[category_idx] + "/" + dialogue_first_words[dialogue_idx]);
      }
      if (leftButton.stateChanged() && leftButton.read() == LOW) {
        // select image
        if (fileArray[file_idx] == "regresar") {
          selectImageIn("main/" + categories[category_idx] + "/" + dialogue_first_words[dialogue_idx], false);
          delay(350);
        } else {
          selectImageIn("main/" + categories[category_idx] + "/" + dialogue_first_words[dialogue_idx], true);
          delay(1300);
        }
        // go back to first word options
        dialogue_first_word = true;
        dialogue_tmp_idx = 0, dialogue_screen_idx = dialogue_idx;
        displayCategories(&dialogue_first_words, dialogue_count, dialogue_tmp_idx, dialogue_screen_idx);
      }
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
      selectImageIn("main/" + categories[category_idx], true);
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