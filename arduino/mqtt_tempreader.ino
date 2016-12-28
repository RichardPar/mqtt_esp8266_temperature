/*
MQTT Wifi Client for nodeMCU

Dallas temperature sensor connected to Pin D4

Listening on MQTT topic inbox for messages - it then replies to 

SENSOR/[IP ADDRESS]/TEMPERATURE   with the current temperature in Celcius

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Update these with values suitable for your network.

const char* ssid = "............";
const char* password = ".................";
const char* mqtt_server = "sdr";
int  lasttemp;
float currenttemp;
char clientasciimac[13];

#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  WiFi.mode(WIFI_STA);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  sensors.begin();
  lasttemp=0;
}

void setup_wifi() {
  uint8_t mac[6];

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.macAddress(mac);
  sprintf(clientasciimac,"%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
 
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  char topicStr[40];
  String ip;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  dtostrf(currenttemp, 2, 2,msg);
  Serial.println(msg);
  ip = WiFi.localIP().toString();
  sprintf(topicStr,"SENSOR/%s/TEMPERATURE",ip.c_str());
  client.publish(topicStr, msg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientasciimac)) {
      sensors.requestTemperatures();
      currenttemp = sensors.getTempCByIndex(0);
      Serial.println("connected");
      client.subscribe("inbox");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}


void loop() {

  char topic[40];
  String ip;
 

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  long now = millis();
  if (now - lastMsg > 15000) {
    lastMsg = now;
    ++value;
   
    sensors.requestTemperatures();
    currenttemp = sensors.getTempCByIndex(0);
    if ((int)(currenttemp*100) != lasttemp)
     {
       lasttemp=(int)(currenttemp*100);
       dtostrf(currenttemp, 2, 2,msg);
       Serial.println(msg);
       ip = WiFi.localIP().toString();
       
       sprintf(topic,"SENSOR/%s/TEMPERATURE",ip.c_str());
       client.publish(topic, msg);
     }
    
    }
}
