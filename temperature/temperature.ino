#include "esp_timer.h"
#include "secrets.h"

#include "WiFi.h"
#include <PubSubClient.h>

#define TOPIC "sensors"

const char* ssid = SECRET_SSID;
const char* password =  SECRET_WIFI_PASS;
const char* mqtt_server = "s3.hyteck.de";
#define MQTT_SERIAL_PUBLISH_CH "/sensors/"
WiFiClient espClient;
PubSubClient client(espClient);


// GadgetBle workflow
static double lastMmntTime = 0;
static double lastRestartTime = 0;
static float mmntIntervalUs = 5000000;
static double restartIntervalUs = 60*60*1000000.0;
static double calibrationTimeUs = 30*1000000.0; // should be alt least 3 min
static double startTime = 0; 
static int64_t mmntIndex = 0;

bool calibrationNeeded = false;

int errorCounter= 0;


void disconnect() {
  WiFi.disconnect();
}

void restart() {
  log("Restarting");
  ESP.restart();
}
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    if(WiFi.status() ==4){
      Serial.println("Connection failed");
    }
    if(WiFi.status() == 6){
      Serial.println("Disconnected");
    }
    if(WiFi.status() == 1){
      Serial.print(ssid);
      Serial.println(":SSID not available");
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), SECRET_BROKER_USERNAME, SECRET_BROKER_PASSWORD)) {
      Serial.println("connected");
      Serial.print("Connection status:");
      Serial.println(client.connected());
      delay(1000);
    } else {
      errorCounter += 1;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      if (client.state() ==2) {
        Serial.print(" Connection refused, ");
      }
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void initMQTTPush(){
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  reconnect();
}


void setup() {
  lastRestartTime = esp_timer_get_time();
  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial);
  initMQTTPush();

  // output format
  Serial.println("Temperature(degC)");
  

  startTime = esp_timer_get_time();
  Serial.println("Ending setup");
}

void loop() {
  if (errorCounter > 10) {
    Serial.println("errorCounter > 10: Restarting");
    disconnect();
    initMQTTPush();
  }
  if (esp_timer_get_time() - lastMmntTime >= mmntIntervalUs) {
    measure_and_report();
  }
  if (esp_timer_get_time() - lastRestartTime >= restartIntervalUs) {
    restart();
  }

  delay(3);
}

void log(String message){
  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;
  
  time(&now);
  // Set timezone to China Standard Time
  setenv("TZ", "CST-8", 1);
  tzset();
  
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%H:%M:%S", &timeinfo);
  String log_message = "[";
  log_message += strftime_buf;
  log_message += "] ";
  log_message += message;
  Serial.println(log_message);
}

void measure_and_report() {
  mmntIndex += 1;
  float co2, temperature, humidity;
  uint8_t data[12], counter;

  
  temperature = 12;
  // Write values to serial
  Serial.println(temperature);

  // MQTT publish
  lastMmntTime = esp_timer_get_time();
  if (!client.connected()) {
    reconnect();
  }
  
  char temp_char[8]; // Buffer big enough for 7-character float
  dtostrf(temperature, 6, 2, temp_char);
  if (not(client.publish("sensors/temperature2", temp_char))){
    Serial.println("Temperature Publish failed");
  }
 

  char mmntIndex_char[20];
  dtostrf(mmntIndex, 6, 2, mmntIndex_char);
  if (not(client.publish("sensors/mmntIndex", mmntIndex_char))){
    Serial.println("mmntIndex publish failed");
  }
}
