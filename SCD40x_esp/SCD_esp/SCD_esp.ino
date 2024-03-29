#include "esp_timer.h"
#include "Sensirion_GadgetBle_Lib.h"
#include "secrets.h"

#include <Wire.h>

#include "WiFi.h"
#include <PubSubClient.h>

#define TOPIC "sensors"

const char* ssid = SECRET_SSID;
const char* password =  SECRET_WIFI_PASS;
const char* mqtt_server = "check.hyteck.de";
#define MQTT_SERIAL_PUBLISH_CH "/sensors/"
WiFiClient espClient;
PubSubClient client(espClient);

// SCD41
const int16_t SCD_ADDRESS = 0x62;

// GadgetBle workflow
static int64_t lastMmntTime = 0;
static int mmntIntervalUs = 5000000;
static int calibrationTimeUs = 30*1000000; // should be alt least 3 min
static int64_t startTime = 0; 
GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_CO2);

bool calibrationNeeded = false;



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
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {
  Serial.begin(460800);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  reconnect();

  // Initialize the GadgetBle Library
  gadgetBle.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(gadgetBle.getDeviceIdString());

  // output format
  Serial.println("CO2(ppm)\tTemperature(degC)\tRelativeHumidity(percent)");
  
  // init I2C
  Wire.begin();

  // wait until sensors are ready, > 1000 ms according to datasheet
  delay(1000);
  
  // start scd measurement in periodic mode, will update every 2 s
  Wire.beginTransmission(SCD_ADDRESS);
  //start_periodic_measurement split in two write commands
  Wire.write(0x21);
  Wire.write(0xb1);
  Wire.endTransmission();

  // wait for first measurement to be finished
  delay(2000);
  startTime = esp_timer_get_time();
  Serial.println("Ending setup");
}

void loop() {
  if(calibrationNeeded && esp_timer_get_time() - startTime >= calibrationTimeUs) {
    calibrate();
    calibrationNeeded = false;
  }
  if (esp_timer_get_time() - lastMmntTime >= mmntIntervalUs) {
    measure_and_report();
  }

  gadgetBle.handleEvents();
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

void calibrate() {
  log("Start calibration");
  log("Stopping measurements");
  Wire.beginTransmission(SCD_ADDRESS);
  //stop_periodic_measurement split in two write commands
  Wire.write(0x3f);
  Wire.write(0x86);
  Wire.endTransmission();
  delay(500);

  
  log("Force recalibration");
  Wire.beginTransmission(SCD_ADDRESS);
  //perform_forced_recalibration split in two write commands
  Wire.write(0x36);
  Wire.write(0x2f);
  Wire.endTransmission();
  delay(400);

  
  log("Starting measurements");
  Wire.beginTransmission(SCD_ADDRESS);
  //start_periodic_measurement split in two write commands
  Wire.write(0x21);
  Wire.write(0xb1);
  Wire.endTransmission();
  //Wait for first measurement to be finished
  delay(2000);

}

void measure_and_report() {
  float co2, temperature, humidity;
  uint8_t data[12], counter;

  // send read data command
  Wire.beginTransmission(SCD_ADDRESS);
  Wire.write(0xec);
  Wire.write(0x05);
  Wire.endTransmission();
  
  // read measurement data: 2 bytes co2, 1 byte CRC,
  // 2 bytes T, 1 byte CRC, 2 bytes RH, 1 byte CRC,
  // 2 bytes sensor status, 1 byte CRC
  // stop reading after 12 bytes (not used)
  // other data like  ASC not included
  Wire.requestFrom(SCD_ADDRESS, 12);
  counter = 0;
  while (Wire.available()) {
    data[counter++] = Wire.read();
  }
  
  // floating point conversion according to datasheet
  co2 = (float)((uint16_t)data[0] << 8 | data[1]);
  // convert T in degC
  temperature = -45 + 175 * (float)((uint16_t)data[3] << 8 | data[4]) / 65536;
  // convert RH in %
  humidity = 100 * (float)((uint16_t)data[6] << 8 | data[7]) / 65536;

  // Write values to serial
  Serial.print(co2);
  Serial.print("\t");
  Serial.print(temperature);
  Serial.print("\t");
  Serial.print(humidity);
  Serial.println();

  // BLE publish
  gadgetBle.writeCO2(co2);
  gadgetBle.writeTemperature(temperature);
  gadgetBle.writeHumidity(humidity);

  gadgetBle.commit();
  
  // MQTT publish
  lastMmntTime = esp_timer_get_time();
  if (!client.connected()) {
    reconnect();
  }
  char co2_char[8]; // Buffer big enough for 7-character float
  dtostrf(co2, 6, 2, co2_char);
  if (not(client.publish("sensors/co2", co2_char))){
    Serial.println("CO2 Publish failed");
  }
  
  char temp_char[8]; // Buffer big enough for 7-character float
  dtostrf(temperature, 6, 2, temp_char);
  if (not(client.publish("sensors/temperature", temp_char))){
    Serial.println("Temperature Publish failed");
  }
  
  char humidity_char[8]; // Buffer big enough for 7-character float
  dtostrf(humidity, 6, 2, humidity_char);
  if (not(client.publish("sensors/humidity", humidity_char))){
    Serial.println("Humidity Publish failed");
  }
}
