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


#define DHT22 1

#ifdef DHT22
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN            D3         
#define DHTTYPE           DHT22    

DHT_Unified dht(DHTPIN, DHTTYPE);
sensor_t sensor;

#endif



// Update these with values suitable for your network.

const char* ssid = "............";
const char* password = ".............";
const char* mqtt_server = "sdr";
int  lasttemp;
int lasttemp2;
float currenttemp;
float currenttemp2;
int lasthumidity;
float currenthumidity;

char clientasciimac[20];

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
  
#ifdef DHT22  
  dht.begin();
  dht.temperature().getSensor(&sensor);
#endif
  
  lasttemp=0;
  lasttemp2=0;
  lasthumidity=0;
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
  sprintf(clientasciimac,"%d%d%d%d%d%d",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  Serial.println(clientasciimac);
  //sprintf(clientasciimac,"CLIENT009");
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

#ifdef DHT22
  dtostrf(currenttemp2, 2, 2,msg);
  sprintf(topicStr,"SENSOR/%s/TEMPERATURE2",ip.c_str());
  client.publish(topicStr, msg);

  dtostrf(currenthumidity, 2, 2,msg);
  sprintf(topicStr,"SENSOR/%s/HUMIDITY",ip.c_str());
  client.publish(topicStr, msg);
#endif

  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientasciimac)) {
      delay(200);
      sensors.requestTemperatures();
      currenttemp = sensors.getTempCByIndex(0);
#ifdef DHT22
      currenttemp2 = getDHT22Temperature();
      currenthumidity = getDHT22Humidity();
#endif      
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


#ifdef DHT22

float getDHT22Temperature()
{
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    return 0;
  }

  return event.temperature;
}


float getDHT22Humidity()
{
  sensors_event_t event;  

 dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    return 0;
  }
  
   return event.relative_humidity;
}

#endif  

void loop() {

  char topic[40];
  String ip;
 

  if (!client.connected()) {
    Serial.println("Client not connected... trying again");
    delay(5000);
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

#ifdef DHT22
    currenttemp2 = getDHT22Temperature();
    if ((int)(currenttemp2*100) != lasttemp2)
     {
       lasttemp2=(int)(currenttemp2*100);
       dtostrf(currenttemp2, 2, 2,msg);
       Serial.println(msg);
       ip = WiFi.localIP().toString();
       sprintf(topic,"SENSOR/%s/TEMPERATURE2",ip.c_str());
       client.publish(topic, msg);
     }

    currenthumidity = getDHT22Humidity();
    if ((int)(currenthumidity*100) != lasthumidity)
     {
       lasthumidity=(int)(currenthumidity*100);
       dtostrf(currenthumidity, 2, 2,msg);
       Serial.println(msg);
       ip = WiFi.localIP().toString();
       sprintf(topic,"SENSOR/%s/HUMIDITY",ip.c_str());
       client.publish(topic, msg);
     }
#endif    
    }
}
