### Introduction to Functions

**setup()**: Initializes the hardware and BLE (Bluetooth Low Energy) for the ESP32 device. It sets up the serial communication, starts the SPIFFS file system, initializes BLE with a specific service and characteristic, and sets up callbacks for receiving data. It also initializes the EPD (Electronic Paper Display) and displays the initial price interface.

**loop()**: The main loop function that continuously checks for updates to the main user interface.

**ble_pic()**: Processes BLE data received and saves it to a file. It checks if data has been received completely, writes the data to a file on SPIFFS, and displays the received image on the EPD.

**clear_all()**: Clears the content of the EPD display by initializing the fast mode of the e-ink screen, clearing the screen, and updating the display.

**main_ui()**: The main user interface processing function that calls the `ble_pic()` function to process BLE data.

**UI_price()**: Displays the price information on the EPD. It checks for the existence of specific files on SPIFFS, reads their content, and displays the images on the EPD.