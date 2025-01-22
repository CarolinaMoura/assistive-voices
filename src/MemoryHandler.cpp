#include "MemoryHandler.h"
#include "Config.h"

/**
 * @brief Get the amount of free memory on the device.
 * @return Amount of free memory in bytes.
 */
int getFreeMemory()
{
  extern int *__brkval;
  extern char __bss_end;

  int free_memory;
  if ((int)__brkval == 0)
  {
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  }
  else
  {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  return free_memory;
}

/**
 * @brief Initialize the SD card.
 */

void initializeSD()
{
  Serial.print("Initializing SD card...");

  if (!SD.begin(CHIP_SELECT))
  {
    Serial.println("SD card initialization failed!");
    return;
  }

  Serial.println("SD card initialized.");
}