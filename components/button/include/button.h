#ifndef BUTTON_H
#define BUTTON_H

/**
 * enum with the possible buttons
 */
typedef enum BUTTON {
    LEFTBUTTON,
    RIGHTBUTTON,
} BUTTON;

/**
 * setup the gpios for the buttons
 */
void initButtons();

/**
 * Get the last value for a specific button. If there is no data for 30 seconds it will be aborted.
 * 
 * @param selectedButton        specify a specific button
 * @return                      High or Low for the button output 
 */
int getButton(BUTTON *selectedButton);

/**
 * Clear the Queue with stored button states
 */
void clearStoredButtons();

#endif