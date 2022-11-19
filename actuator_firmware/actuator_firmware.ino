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

const char* ssid = "Semasinghe";
const char* password = "4500322*";
const char* mqtt_server = "test.mosquitto.org";

int co2raw = 0;                               //int for raw value of co2
int co2ppm = 0;
int grafX = 0;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  //Serial.println(messageTemp);
  co2raw = messageTemp.toInt();
  Serial.println(co2raw);
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

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("AMeasure/sensor1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
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
