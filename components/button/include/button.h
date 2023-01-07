#ifndef BUTTON_H
#define BUTTON_H

typedef enum BUTTON {
    LEFTBUTTON,
    RIGHTBUTTON,
} BUTTON;

void initButtons();
int getButton(BUTTON *selectedButton);
void clearStoredButtons();

#endif