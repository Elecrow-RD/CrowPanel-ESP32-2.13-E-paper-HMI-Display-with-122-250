#include <Arduino.h>  // Include Arduino library for basic functionality
#include "EPD.h"     // Include e-paper display library for controlling the e-paper display
#include <WiFi.h>     // Include WiFi library for WiFi connection

// Define SSID and password for the WiFi network.
String ssid = "yanfa_software";
String password = "yanfa-123456";

// Define a black and white image array as a buffer for e-paper display.
extern uint8_t ImageBW[ALLSCREEN_BYTES];

// Setup function.
void setup() {
    // Initialize serial communication at 115200 baud rate.
    Serial.begin(115200);

    // Set the screen power pin as output and turn on the power.
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    // Start connecting to WiFi.
    WiFi.begin(ssid, password);

    // Wait until WiFi is connected.
    while (WiFi.status()!= WL_CONNECTED) {
        delay(500); // Check connection status every 500 milliseconds.
        Serial.print("."); // Display progress dots on serial monitor.
    }

    // After WiFi connection is established, print information.
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); // Print the device's IP address.

    // Create character arrays for displaying information.
    char buffer[40];
    char buffer1[40];
    EPD_Init();
    EPD_ALL_Fill(WHITE);
    EPD_Update();
    EPD_Clear_R26H();

    // Set and display text for WiFi connection status.
    strcpy(buffer, "WiFi connected");
    EPD_ShowString(0, 0 + 0 * 20, buffer, BLACK, 16);

    // Set and display text for IP address label.
    strcpy(buffer, "IP address: ");
    strcpy(buffer1, WiFi.localIP().toString().c_str());
    EPD_ShowString(0, 0 + 1 * 20, buffer, BLACK, 16);
    EPD_ShowString(0, 0 + 2 * 20, buffer1, BLACK, 16);

    EPD_DisplayImage(ImageBW);
    EPD_PartUpdate();
    EPD_Sleep();
}

// Main loop function.
void loop() {
    // No functionality implemented in the main loop.
    delay(10); // Wait for 10 milliseconds.
}
