#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "EPD.h"
#include "Pic.h"

// Define an array to store black and white image data for the e-paper display buffer.
// This array is likely used to hold pixel data for the display.
extern uint8_t ImageBW[ALLSCREEN_BYTES];

// WiFi network name.
const char* ssid = "yanfa_software";
// WiFi password.
const char* password = "yanfa-123456";

// OpenWeatherMap API key. This key is used to access weather data from the OpenWeatherMap API.
String openWeatherMapApiKey = "You-API";

// City and country code for which weather data is to be fetched.
String city = "London";
String countryCode = "2643743";

// Variables for timing and API calls.
// lastTime is used to keep track of the last time an API call was made.
unsigned long lastTime = 0;
// timerDelay determines the time interval between API calls.
unsigned long timerDelay = 10000;

// Variables for JSON data.
// jsonBuffer stores the raw JSON data received from the API.
String jsonBuffer;
// httpResponseCode holds the response code from the HTTP request.
int httpResponseCode;
// myObject is a JSON variable that will hold the parsed JSON data.
JSONVar myObject;

// Variables for weather information.
// weather stores the main weather description.
String weather;
// temperature stores the temperature value.
String temperature;
// humidity stores the humidity value.
String humidity;
// sea_level stores the sea level value.
String sea_level;
// wind_speed stores the wind speed value.
String wind_speed;
// city_js stores the name of the city.
String city_js;
// weather_flag is a flag that determines which weather icon to display based on the weather description.
int weather_flag = 0;

// Function to display weather forecast on the e-paper display.
void UI_weather_forecast()
{
    // buffer is used to hold formatted strings for display on the e-paper.
    char buffer[40];

    // Initialize the e-paper display.
    EPD_Init();
    // Fill the entire display with white color.
    EPD_ALL_Fill(WHITE);
    // Update the display.
    EPD_Update();
    // Clear the display using a specific method.
    EPD_Clear_R26H();

    // Display a background picture on the e-paper.
    EPD_ShowPicture(0, 0, 248, 122, pic, BLACK);
    // Display the weather icon based on the weather_flag.
    EPD_ShowPicture(1, 4, 144, 80, Weather_Num[weather_flag], BLACK);

    // Display the city name on the e-paper.
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s ", city_js);
    EPD_ShowString(190, 30, buffer, BLACK, 12);

    // Display the temperature on the e-paper.
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s C", temperature);
    EPD_ShowString(110, 110, buffer, BLACK, 12);

    // Display the humidity on the e-paper.
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s ", humidity);
    EPD_ShowString(200, 72, buffer, BLACK, 12);

    // Display the wind speed on the e-paper.
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s m/s", wind_speed);
    EPD_ShowString(30, 110, buffer, BLACK, 12);

    // Display the sea level on the e-paper.
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s ", sea_level);
    EPD_ShowString(210, 110, buffer, BLACK, 12);

    // Update the e-paper display with the new content.
    EPD_DisplayImage(ImageBW);
    // Perform a partial update of the e-paper display.
    EPD_PartUpdate();
    // Put the e-paper display into sleep mode to conserve power.
    EPD_Sleep();
}

void setup() {
    // Initialize the serial communication at 115200 baud rate.
    Serial.begin(115200);

    // Connect to the WiFi network.
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    // Wait until the connection is established.
    while (WiFi.status()!= WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    // Print the IP address of the device on the connected WiFi network.
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");

    // Set pin 7 as an output pin.
    pinMode(7, OUTPUT);
    // Set pin 7 to high to enable power to the e-paper display.
    digitalWrite(7, HIGH);
}

void loop() {
    // Call the function to analyze the JSON data received from the API.
    js_analysis();
    // Call the function to update the e-paper display with the weather forecast.
    UI_weather_forecast();
    // Delay for one hour before the next iteration of the loop.
    delay(1000 * 60 * 60);
}

void js_analysis()
{
    // Check if the device is connected to the WiFi network.
    if (WiFi.status() == WL_CONNECTED) {
        // Construct the URL for the OpenWeatherMap API request.
        String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";

        // Keep trying until a successful response (status code 200) is received.
        while (httpResponseCode!= 200) {
            // Make an HTTP GET request to the API and store the response in jsonBuffer.
            jsonBuffer = httpGETRequest(serverPath.c_str());
            Serial.println(jsonBuffer);
            // Parse the JSON data.
            myObject = JSON.parse(jsonBuffer);

            // Check if the JSON parsing was successful.
            if (JSON.typeof(myObject) == "undefined") {
                Serial.println("Parsing input failed!");
                return;
            }
            // Wait for 2 seconds before retrying.
            delay(2000);
        }

        // Extract weather information from the parsed JSON data.
        weather = JSON.stringify(myObject["weather"][0]["main"]);
        temperature = JSON.stringify(myObject["main"]["temp"]);
        humidity = JSON.stringify(myObject["main"]["humidity"]);
        sea_level = JSON.stringify(myObject["main"]["sea_level"]);
        wind_speed = JSON.stringify(myObject["wind"]["speed"]);
        city_js = JSON.stringify(myObject["name"]);

        // Print the extracted weather information for debugging purposes.
        Serial.print("String weather: ");
        Serial.println(weather);
        Serial.print("String Temperature: ");
        Serial.println(temperature);
        Serial.print("String humidity: ");
        Serial.println(humidity);
        Serial.print("String sea_level: ");
        Serial.println(sea_level);
        Serial.print("String wind_speed: ");
        Serial.println(wind_speed);
        Serial.print("String city_js: ");
        Serial.println(city_js);

        // Determine the weather icon based on the weather description.
        if (weather.indexOf("clouds")!= -1 || weather.indexOf("Clouds")!= -1 ) {
            weather_flag = 1;
        } else if (weather.indexOf("clear sky")!= -1 || weather.indexOf("Clear sky")!= -1 ||weather.indexOf("Clear")!= -1) {
            weather_flag = 3;
        } else if (weather.indexOf("rain")!= -1 || weather.indexOf("Rain")!= -1) {
            weather_flag = 5;
        } else if (weather.indexOf("thunderstorm")!= -1 || weather.indexOf("Thunderstorm")!= -1) {
            weather_flag = 2;
        } else if (weather.indexOf("snow")!= -1 || weather.indexOf("Snow")!= -1) {
            weather_flag = 4;
        } else if (weather.indexOf("mist")!= -1 || weather.indexOf("Mist")!= -1) {
            weather_flag = 0;
        }
    }
    else {
        // Print a message if the device is not connected to the WiFi network.
        Serial.println("WiFi Disconnected");
    }
}

// Function to make an HTTP GET request and return the response.
String httpGETRequest(const char* serverName) {
    // Create a WiFiClient object.
    WiFiClient client;
    // Create an HTTPClient object.
    HTTPClient http;

    // Initialize the HTTP client with the server name.
    http.begin(client, serverName);

    // Send an HTTP GET request.
    httpResponseCode = http.GET();

    // Initialize the response payload as an empty JSON object.
    String payload = "{}";

    // Check if the response code is positive (indicating a successful request).
    if (httpResponseCode > 0) {
        // Print the response code for debugging purposes.
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        // Get the response as a string and store it in payload.
        payload = http.getString();
    }
    else {
        // Print the error code if the request was not successful.
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }

    // Close the HTTP connection.
    http.end();

    // Return the response payload.
    return payload;
}
