#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define co2Zero     55                        //calibrated CO2 0 level
#define OLED_RESET   4                        //OLED reset on lin 4

const char *WIFI_SSID = "YOUR WIFI NETWORK NAME";
const char *WIFI_PASSWORD = "YOUR WIFI PASSWORD";

const char *MQTT_HOST = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char *TOPIC = "AMeasure/sensor1";

int co2raw = 0;                               //int for raw value of co2
int co2ppm = 0;
int grafX = 0;

WiFiClient client;
PubSubClient mqttClient(client);

void callback(char* topic, byte* payload, unsigned int length)
{
  payload[length] = '\0';
  co2raw = String((char*) payload).toInt();
  co2ppm = co2raw - co2Zero;                 //get calculated ppm
  display.setTextSize(2);                     //set text sizez
  display.setTextColor(WHITE);                //set text color
  display.setCursor(0, 0);                    //set cursor
  display.println("CO2 Level");               //print title
  display.println(" ");                       //skip a line
  display.print(co2ppm);
  display.print("  PPM");
  Serial.print("AirQuality=");
  //display.setCursor(20,27);
  Serial.print(co2ppm);  // prints the value read
  Serial.println(" PPM");
  delay(50);
  grafX = map(co2ppm, 200, 1023, 0, 127);       //map value to screen width
  display.fillRect(0, 52, grafX, 10, SSD1306_WHITE);  //print graph 400min 1000max
  display.display();
}

void setup()
{
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to Wi-Fi");

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(callback);
  mqttClient.subscribe(TOPIC);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  testscrolltext();
  display.clearDisplay();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}

void testscrolltext(void) {
  display.clearDisplay();
  int16_t i = 0;
  display.drawRect(i, i, display.width(), display.height(), SSD1306_WHITE);
  display.display(); // Update screen with each newly-drawn rectangle
  display.setTextSize(1.5); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 27);
  display.display();      // Show initial text
  display.clearDisplay();
  delay(3000);

}

void WarmingSensor(void) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  int16_t i = 0;
  display.drawRect(i, i, display.width(), display.height(), SSD1306_WHITE);
  display.setTextSize(2); // Draw 2X-scale text
  display.setCursor(15, 27);
  display.println(F("CO2 Meter"));
  display.display(); // Show initial text
  display.clearDisplay();
  delay(3000);
  display.display();
  for (int i = 0; i <= 100; i++)
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    int16_t j = 0;
    display.drawRect(j, j, display.width(), display.height(), SSD1306_WHITE);
    display.setCursor(20, 27);
    display.setTextSize(1.5);
    display.println(F("Warming Sensor:"));
    display.setTextSize(1.5);
    display.setCursor(20, 37);
    if (i < 100) display.print("");
    if (i < 10) display.print("");
    display.print(i);
    display.print("%");
    display.display();
    delay(1000);
  }
}
