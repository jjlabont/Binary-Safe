#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED_PIN0 (1 << PB0)
#define LED_PIN1 (1 << PB1)
#define LED_PIN2 (1 << PB2)
#define KEY_PIN0 (1 << PB3)
#define KEY_PIN1 (1 << PB4)

volatile int keyPressed = 0;
volatile int isLocked = 0;
volatile uint8_t lockCode = 0;
volatile uint8_t input = 0;
volatile uint8_t history = 0;
volatile int numDigitsEntered = 0;
volatile int isPressed = 0;

//lights green led
void green() {
    DDRB = LED_PIN0|LED_PIN1;
    PORTB = LED_PIN0;
}

//lights yellow led
void yellow() {
    DDRB = LED_PIN0|LED_PIN1;
    PORTB = LED_PIN1;
}

//lights blue led
void blue() {
    DDRB = LED_PIN1|LED_PIN2;
    PORTB = LED_PIN1;
}

//lights red led
void red() {
    DDRB = LED_PIN1|LED_PIN2;
    PORTB = LED_PIN2;
}

//lights green and yellow leds
void grellow() {
    green();
    yellow();
}

//flashes yellow led
void error() {
    yellow();
    _delay_ms(250);
    PORTB = 0;
    _delay_ms(250);
     yellow();
    _delay_ms(250);
    PORTB = 0;
    _delay_ms(250);
    yellow();
    _delay_ms(250);
    PORTB = 0;
}

ISR(TIMER0_COMPA_vect) {
    //shift histroy left
    history = history << 1;
    //if pin 0 is down
    if (!(PINB & KEY_PIN0)) {
        history = history | 0x1; // if butotns is push add 1 to end of history
        keyPressed = 0; // set key pressed to 0
    }
    //if pin 1 is down
    if (!(PINB & KEY_PIN1)) {
        history = history | 0x1; // if buttons is push add 1 to end of history
        keyPressed = 1;
    }
        
    //if last 6 inputs were a button held down
    if ((history == 0b111111) && (isPressed == 0)) {
        blue();
        _delay_ms(10);
        //left shift input
        input = input << 1;
        //increment the amount of digits entered
        numDigitsEntered += 1;
        //if the press was a 1 OR it to the back
        if (keyPressed == 1) {
            input = input | 0b1;
        }
            //button has been pressed
            isPressed = 1;
    }

    //if not locked and 6 digits have been entered
    if (numDigitsEntered == 6) {
        //if it isnt locked, set input as lockCode and reset input to 0 and lock
        if (!isLocked) {
            isLocked = 1;
            numDigitsEntered = 0;
            lockCode = input;
            input = 0;
        } else if (isLocked) { //else if its locked and 6 digits have been entered
            if (input == lockCode) { // if input matches lockcode, unlock and reset num digits entered
                isLocked = 0;
                numDigitsEntered = 0;
                input = 0;
            } else { // reset input and digits entered and flashes error light
                error(); //flashes error lights
                input = 0;
                numDigitsEntered = 0;
            }
        }
    } 
    //if button has been released
    if (history == 0b000000) {
        isPressed = 0;
    }
}

int main(void) {
    TCCR0B = 0b1;
    TIMSK = (1 << OCIE0A); //enable timer comparator interupt
    sei(); //enable interupts

    while (1) {
        //while the safe is not locked light green and yellow LEDs
        while (!isLocked) {
            grellow();
        }
        //while the safe is locked light red LED
        while (isLocked) {
            red();
        }
    }
}