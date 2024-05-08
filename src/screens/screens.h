#ifndef SCREENS_H
#define SCREENS_H

#ifndef GLOBALS_H
#include "globals.h"
#endif

#ifndef TOUCH_H
#include <touch.h>
#endif

class ScreenObject
{
private:
    std::vector<std::array<int, 4>> longPressCoordArray;
    std::vector<std::array<int, 4>> swipeCoordArray;
    std::vector<std::array<int, 4>> pressCoordArray;

    std::vector<void (*)()> longPressActions;
    std::vector<void (*)()> swipeActions;
    std::vector<void (*)()> pressActions;

public:
    ScreenObject(std::vector<std::array<int, 4>> LPC = {}, std::vector<std::array<int, 4>> SC = {}, std::vector<std::array<int, 4>> PC = {},
                 std::vector<void (*)()> LPA = {}, std::vector<void (*)()> SA = {}, std::vector<void (*)()> PA = {})
    {
        longPressCoordArray = LPC;
        swipeCoordArray = SC;
        pressCoordArray = PC;
        longPressActions = LPA;
        swipeActions = SA;
        pressActions = PA;
    }

    void processTouch()
    {

        std::vector<std::array<int, 4>> coords;
        std::vector<void (*)()> actions;
        int x, y, devX, devY;
        x = touchCurrentAction[1];
        y = touchCurrentAction[2];

        // Select arrays to corresponding touch type
        switch (touchCurrentAction[0])
        {
            {
            case 0:
                coords = longPressCoordArray;
                actions = longPressActions;
                break;
            case 1:
                coords = swipeCoordArray;
                actions = swipeActions;
                break;
            case 2:
                coords = pressCoordArray;
                actions = pressActions;
                break;
            }
        }

        for (int i = 0; i < coords.size(); i++)
        {
            if (coords[i][0] < x && x < coords[i][2] && coords[i][1] < y && y < coords[i][3])
            {
                actions[i]();
                break;
            }
        }
    }
};

extern ScreenObject *activeScreenElement;
extern TaskHandle_t updateDisplay_t;

void displaySleep();
void menuScreen();
void clockScreen();
void test1Screen();
void test2Screen();
void testDirection();
void updateDisplay(void *params);
void keyboardPopUp(void *params);

void drawKeyboard(coordinates &coords1, coordinates &coords2);

#endif