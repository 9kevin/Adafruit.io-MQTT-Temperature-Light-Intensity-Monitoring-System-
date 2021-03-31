// Adafruit.io MQTT Temperature & Light Intensity Monitoring System 
#include <ESP8266WiFi.h> 
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h" 
#include "DHT.h" 
/** WiFi Access Point **/ 
#define Show(string,val) Serial.print(string); Serial.println(val); #define WLAN_SSID "MY_SSID" 
#define WLAN_PASS "MY_PASS" 
/** Adafruit.io Setup **/ 
#define AIO_SERVER "io.adafruit.com" 
#define AIO_SERVERPORT 1883 // 8883 for SSL #define AIO_USERNAME "MY_AIO_USERNAME" 
#define AIO_KEY "MY_AIO_KEY" 
/** Global State **/ 
// DHT Sensor 
#define DHTTYPE DHT11 // DHT 11 
//#define DHTTYPE DHT21 // DHT 21 (AM2301) 
//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321 #define DHT_PIN 15 // GPIO15/D8 
// LDR Light Intensity Sensor 
#define LDR_PIN A0 // ADC0/A0/17 
#define LED_LIGHT_PIN 5 // GPIO5/D1 
#define FAN_PIN 13 // GPIO13/D7

// Initialize DHT Sensor 
DHT dht(DHT_PIN, DHTTYPE); 
float temperature_value=0; 
float light_intensity_value=0; 
// ESP8266 WiFiClient class to connect to the MQTT server WiFiClient client; 
// or... WiFiClientSecure for SSL 
// WiFiClientSecure client; 
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY); 
/** Feeds **/ 
// Publishing Feeds: 
// Temperature feed 'roomtemperature' 
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/roomtemperature"); 
// Light Intensity feed 'roomlightintensity' 
Adafruit_MQTT_Publish lightintensity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/roomlightintensity"); 
// Subscribe to Changes: 
// Fan on/off button feed 'roomfan' 
Adafruit_MQTT_Subscribe fanonoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/roomfan"); 
// LED Light on/off button feed 'roomledlight' 
Adafruit_MQTT_Subscribe ledlightonoffbutton = 
Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/roomledlight"); void MQTT_connect();
//ESP8266 and Adafruit IO 
void setup() { 
Serial.begin(115200); 
delay(100); 
Serial.println(F("Adafruit MQTT Home Temperature & Light Intensity Monitor")); 
// LED LIGHT 
pinMode(LED_LIGHT_PIN, OUTPUT); // LED pin as output // FAN 
pinMode(FAN_PIN, OUTPUT); 
// DHT Sensor 
pinMode(DHT_PIN, INPUT); 
dht.begin(); 
// LDR Sensor 
pinMode(LDR_PIN, INPUT); 
// connect to WiFi access point 
Serial.println();Serial.println(); 
Serial.print("Connecting to "); 
Serial.println(WLAN_SSID); 
WiFi.begin(WLAN_SSID, WLAN_PASS); 
while (WiFi.status() != WL_CONNECTED) { 
delay(500); 
Serial.print("."); 
} 
Serial.println(); 
Serial.println("WiFi connected"); 
Serial.println("IP address: ");Serial.println(WiFi.localIP()); 
// Setup MQTT subscription for onoff feeds. 
mqtt.subscribe(&fanonoffbutton); 
mqtt.subscribe(&ledlightonoffbutton); 
}
//ESP8266 and Adafruit IO 
void loop() { 
// Ensure the connection to the MQTT server is alive (this will make the first 
// connection and automatically reconnect when disconnected) MQTT_connect(); 
// wait for incoming subscription packets 
Adafruit_MQTT_Subscribe *subscription; 
while ((subscription = mqtt.readSubscription(5000))) { 
if (subscription == &fanonoffbutton) { 
Serial.print(F("FAN Went: ")); 
Serial.println((char *)fanonoffbutton.lastread); 
String readfanonoffbutton=(char *)fanonoffbutton.lastread; Show("FAN_BUTTON_STATE= ",readfanonoffbutton); 
if (readfanonoffbutton.indexOf("ON")>=0){ 
digitalWrite(FAN_PIN, 0); 
delay (2000); // simulated time to complete operation, will show delay in dashboard 
} else { 
digitalWrite(FAN_PIN, 1); 
delay(2000); 
} 
} else if (subscription == &ledlightonoffbutton) { 
Serial.print(F("LED Went: ")); 
Serial.println((char *)ledlightonoffbutton.lastread); 
String readledlightonoffbutton=(char *)ledlightonoffbutton.lastread; Show("LED_LIGHT_BUTTON_STATE= ",readledlightonoffbutton); 
if (readledlightonoffbutton.indexOf("ON")>=0){ 
digitalWrite(LED_LIGHT_PIN, 0); 
delay (2000); 
}else{ 
digitalWrite(LED_LIGHT_PIN, 1); 
delay(2000); 
} 
} 
}
//ESP8266 and Adafruit IO 
// PUBLISH STUFF 
// Publish TEMPERATURE Value 
temperature_value = dht.readTemperature(); // Gets the values of the temperature 
Serial.print(F("\nSending temperature val ")); 
Serial.print(temperature_value); 
Serial.print("..."); 
if (! temperature.publish(temperature_value)) { 
Serial.println(F("Failed")); 
} else { 
Serial.println(F("OK!")); 
} 
Serial.println(); 
// Publish LIGHT INTENSITY Value 
light_intensity_value = analogRead(LDR_PIN); // Gets the values of the light intensity 
Serial.print(F("\nSending light intensity val ")); 
Serial.print(light_intensity_value); 
Serial.print("..."); 
if (! lightintensity.publish(light_intensity_value)) { Serial.println(F("Failed")); 
} else { 
Serial.println(F("OK!")); 
} 
Serial.println(); 
// ping the server to keep the mqtt connection alive 
// NOT required if you are publishing once every KEEPALIVE seconds 
if(! mqtt.ping()) { 
mqtt.disconnect(); 
} 
} 
// Function to connect and reconnect as necessary to the MQTT server. // Should be called in the loop function and it will take care if connecting.
//ESP8266 and Adafruit IO 
void MQTT_connect() { 
int8_t ret; 
// Stop if already connected. 
if (mqtt.connected()) { 
return; 
} 
Serial.print("Connecting to MQTT... "); 
uint8_t retries = 3; 
while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected 
Serial.println(mqtt.connectErrorString(ret)); 
Serial.println("Retrying MQTT connection in 5 seconds..."); mqtt.disconnect(); 
delay(5000); // wait 5 seconds 
retries--; 
if (retries == 0) { 
// basically die and wait for WDT to reset me 
while (1); 
} 
} 
Serial.println("MQTT Connected!"); 
}
