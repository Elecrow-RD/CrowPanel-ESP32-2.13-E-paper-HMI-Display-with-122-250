#include <Arduino.h>
#include "EPD.h"

// Pre-allocated black and white image array.
extern uint8_t ImageBW[ALLSCREEN_BYTES];

// Button definitions.
#define HOME_KEY 2
#define EXIT_KEY 1
#define PRV_KEY 6
#define NEXT_KEY 4
#define OK_KEY 5

// Initialization of button counters.
int HOME_NUM = 0;
int EXIT_NUM = 0;
int PRV_NUM = 0;
int NEXT_NUM = 0;
int OK_NUM = 0;

// Array to store button counts.
int NUM_btn[5] = {0};

// Function to count button presses and display the counts.
void count_btn(int NUM[5])
{
    char buffer[30];

    EPD_Init();
    EPD_ALL_Fill(WHITE);
    EPD_Update();
    EPD_Clear_R26H();

    // Display the count for each button.
    int length = sprintf(buffer, "HOME_KEY_NUM:%d", NUM[0]);
    buffer[length] = '\0';
    EPD_ShowString(0, 0 + 0 * 20, buffer, BLACK, 16);

    length = sprintf(buffer, "EXIT_KEY_NUM:%d", NUM[1]);
    buffer[length] = '\0';
    EPD_ShowString(0, 0 + 1 * 20, buffer, BLACK, 16);

    length = sprintf(buffer, "PRV_KEY_NUM:%d", NUM[2]);
    buffer[length] = '\0';
    EPD_ShowString(0, 0 + 2 * 20, buffer, BLACK, 16 );

    length = sprintf(buffer, "NEXT__NUM:%d", NUM[3]);
    buffer[length] = '\0';
    EPD_ShowString(0, 0 + 3 * 20, buffer, BLACK, 16);

    length = sprintf(buffer, "OK_NUM:%d", NUM[4]);
    buffer[length] = '\0';
    EPD_ShowString(0, 0 + 4 * 20, buffer, BLACK, 16);

    EPD_DisplayImage(ImageBW);
    EPD_PartUpdate();
    EPD_Sleep();
}

// Setup function.
void setup() {
    Serial.begin(115200);

    // Set pin 7 as output for power control.
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    // Set button pins as input.
    pinMode(HOME_KEY, INPUT);
    pinMode(EXIT_KEY, INPUT);
    pinMode(PRV_KEY, INPUT);
    pinMode(NEXT_KEY, INPUT);
    pinMode(OK_KEY, INPUT);
}

// Main loop function.
void loop() {
    int flag = 0;

    // Check if HOME_KEY is pressed.
    if (digitalRead(HOME_KEY) == 0)
    {
        delay(100);
        if (digitalRead(HOME_KEY) == 1)
        {
            Serial.println("HOME_KEY");
            HOME_NUM++;
            flag = 1;
        }
    }
    // Check if EXIT_KEY is pressed.
    else if (digitalRead(EXIT_KEY) == 0)
    {
        delay(100);
        if (digitalRead(EXIT_KEY) == 1)
        {
            Serial.println("EXIT_KEY");
            EXIT_NUM++;
            flag = 1;
        }
    }
    // Check if PRV_KEY is pressed.
    else if (digitalRead(PRV_KEY) == 0)
    {
        delay(100);
        if (digitalRead(PRV_KEY) == 1)
        {
            Serial.println("PRV_KEY");
            PRV_NUM++;
            flag = 1;
        }
    }
    // Check if NEXT_KEY is pressed.
    else if (digitalRead(NEXT_KEY) == 0)
    {
        delay(100);
        if (digitalRead(NEXT_KEY) == 1)
        {
            Serial.println("NEXT_KEY");
            NEXT_NUM++;
            flag = 1;
        }
    }
    // Check if OK_KEY is pressed.
    else if (digitalRead(OK_KEY) == 0)
    {
        delay(100);
        if (digitalRead(OK_KEY) == 1)
        {
            Serial.println("OK_KEY");
            OK_NUM++;
            flag = 1;
        }
    }

    // If a button is pressed, update the display.
    if (flag == 1)
    {
        NUM_btn[0] = HOME_NUM;
        NUM_btn[1] = EXIT_NUM;
        NUM_btn[2] = PRV_NUM;
        NUM_btn[3] = NEXT_NUM;
        NUM_btn[4] = OK_NUM;

        count_btn(NUM_btn);
        flag = 0;
    }
}
