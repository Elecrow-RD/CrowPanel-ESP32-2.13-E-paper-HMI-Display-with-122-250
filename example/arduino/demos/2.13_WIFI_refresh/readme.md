### Introduction to Functions
**clear_all()**:
Initializes the fast mode of the electronic paper display, clears the display content, updates the display, and puts it into deep sleep mode to save power.

**handle_root()**:
Sends the HTML upload page to the client when the root path is requested.

**okPage()**:
Handles file upload requests, saves the uploaded file, and displays the uploaded image on the electronic paper display. It also sends an HTML success page to the client after the upload is complete.

**setup()**:
Sets up the serial communication, initializes the SPIFFS file system, starts the WiFi access point, initializes the HTTP server, sets up the EPD display, and displays the initial price information.

**loop()**:
Handles client requests for the HTTP server.

**UI_price()**:
Displays price information on the EPD. It checks for the existence of specific files on SPIFFS, reads their content, and displays the images on the EPD.