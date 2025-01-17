#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <Arduino.h>

// Button resistance macros
#define LIGHT 30     // Short debounce time, in milliseconds. For buttons without much resistance.
#define MODERATE 50  // Moderate debounce time, in milliseconds. For buttons with moderate resistance.
#define HARD 70      // Longer debounce time, in milliseconds. For buttons with the highest level of resistance.

class Debounce {
private:
    uint8_t pin;           // Button pin 
    unsigned long delay;   // Debounce delay, ms
    unsigned long last_timestamp; // Timestamp of the last valid state change
    int last_state;         // Last button state
    int current_state;      // Current button state

public:
    /**
     * @brief Creates a Debounce object.
     * @param pin The pin associated with the button.
     * @param resistance The debounce time defined by the resistance level (LIGHT, MODERATE, HARD).
     */
    Debounce(uint8_t pin, int resistance);

    /**
     * @brief Initializes the pin for input.
     */
    void begin();

    /**
     * @brief Reads the debounced state of the button.
     * @return The stable state of the button (HIGH or LOW).
     */
    int read();

    /**
     * @brief Checks if the button state has changed.
     * @return True if the state has changed, false otherwise.
     */
    bool stateChanged();
};

#endif // DEBOUNCE_H