#include "Debounce.h"

Debounce::Debounce(uint8_t pin, int resistance)
    : pin( pin ) , last_timestamp( 0 ), last_state( HIGH ), current_state( HIGH ) 

{
    switch( resistance ) {
        case 0: 
            delay = LIGHT ; break ;
        case 1: 
            delay = MODERATE ; break ;
        case 2: 
            delay = HARD ; break ;
    } ;

    Serial.print(F("delay: ")) ;
    Serial.print( delay ) ;
}

void Debounce::begin() {
    pinMode( pin, INPUT );
    last_state = digitalRead( pin );
}

int Debounce::read() {
    int reading = digitalRead( pin );
    if ( reading != current_state ) {
        unsigned long now = millis();
        if ( now - last_timestamp >= delay ) {
            current_state = reading ;
            last_timestamp = now ;
        }
    }
    return current_state ;
}

bool Debounce::stateChanged() {
    int new_state = read() ;
    if ( new_state != last_state ) {
        last_state = new_state ;
        return true;
    }
    return false;
}