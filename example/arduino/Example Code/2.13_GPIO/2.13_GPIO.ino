#include <Arduino.h>
#include "EPD.h"

// Define a black and white image array as a buffer for e-paper display.
extern uint8_t ImageBW[ALLSCREEN_BYTES];

// Define the pin for the home key.
#define HOME_KEY 2

// Variable to count home key presses.
int HOME_NUM = 0;

// Setup function.
void setup() {
    // Initialize serial communication at 115200 baud rate.
    Serial.begin(115200);

    // Set the screen power pin as output and turn on the power.
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    // Set the POWER light pin as output.
    pinMode(40, OUTPUT);
    pinMode(41, OUTPUT);
    // Set the home key pin as input.
    pinMode(HOME_KEY, INPUT);
}

// Main loop function.
void loop() {
    // Flag variable to indicate if a key is pressed.
    int flag = 0;

    // Check if the home key is pressed.
    if (digitalRead(HOME_KEY) == 0) {
        delay(100); // Debounce delay.

        // Check again to ensure stable key state.
        if (digitalRead(HOME_KEY) == 1) {
            Serial.println("HOME_KEY");
            HOME_NUM =!HOME_NUM; // Toggle the home key state.

            flag = 1; // Set the flag to indicate update needed.
        }
    }

    // If a key is pressed, update the display.
    if (flag == 1) {
        char buffer[30];
        char buffer1[30];
        EPD_Init();
        EPD_ALL_Fill(WHITE);
        EPD_Update();
        EPD_Clear_R26H();

        // Set the POWER light and display text based on home key state.
        if (HOME_NUM == 1) {
            digitalWrite(40, HIGH);
            digitalWrite(41, HIGH);
            strcpy(buffer, "GPIO 40:on");
            strcpy(buffer1, "GPIO 41:on");
        } else {
            digitalWrite(40, LOW);
            digitalWrite(41, LOyW);
            strcpy(buffer, "GPIO 40:off");
            strcpy(buffer1, "GPIO 41:off");
        }                   

        // Display the text on the e-paper.
        EPD_ShowString(0, 0 + 0 * 20, "           ", BLACK, 16);
        EPD_ShowString(0, 0 + 1 * 20, "           ", BLACK, 16);
        
        EPD_ShowString(0, 0 + 0 * 20, buffer, BLACK, 16);
        EPD_ShowString(0, 0 + 1 * 20, buffer1, BLACK, 16);;
        EPD_DisplayImage(ImageBW);
        EPD_PartUpdate();
        EPD_Sleep();
    }
}
