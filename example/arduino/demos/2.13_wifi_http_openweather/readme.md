### Introduction to Functions

**setup()**: Initializes the serial communication, connects to the WiFi network, sets up the GPIO for the e-paper display power, initializes the e-paper display, and prints the IP address after successful WiFi connection.

**loop()**: The main loop that calls the `js_analysis()` function to parse weather data and the `UI_weather_forecast()` function to update the weather display on the e-paper. It includes a delay to refresh the display every hour.

**UI_weather_forecast()**: Displays the weather forecast on the e-paper. It clears the display, shows the background image, weather icon, and weather data (temperature, humidity, wind speed, sea level pressure). It then updates the e-paper display and puts it into deep sleep mode to save power.

**js_analysis()**: Connects to the OpenWeatherMap API to fetch weather data for a specified city. It parses the JSON response to extract weather information and sets the appropriate weather icon flag. It also handles reconnection attempts if the HTTP response code is not 200.

**httpGETRequest(const char* serverName)**: Sends an HTTP GET request to the specified server and returns the response content. It handles the HTTP client connection, sends the request, and processes the response, including error handling if the response code is not positive.