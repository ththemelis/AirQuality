#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <PubSubClient.h>
#include "Seeed_HM330X.h" // https://github.com/Seeed-Studio/Seeed_PM2_5_sensor_HM3301
#include "Zanshin_BME680.h"
#include "MutichannelGasSensor.h" // https://github.com/Seeed-Studio/Mutichannel_Gas_Sensor
#include "vars.h"
#include "secrets.h"

unsigned long time_now = 0;

EthernetClient ethClient; // Δημιουργία αντικειμένου για την ενσύρματη σύνδεση στο διαδίκτυο

PubSubClient mqttClient(ethClient);

BME680_Class BME680; // Δημιουργία αντικειμένου για τον αισθητήρα BME680

float altitude(const int32_t press, const float seaLevel = 1013.25); // Forward function declaration with default value for sea level

// Ορισμός παραμέτρων για τον αισθητήρα σωματιδίων
const byte SensorPayloadLength = 28;
const byte SensorPayloadBufferSize = 29;
const byte SensorPayloadPM1_0Position = 4;
const byte SensorPayloadPM2_5Position = 6;
const byte SensorPayloadPM10_0Position = 8;
u8 buf[100]; // Αρχικοποίηση της μεταβλητής η οποία θα περιέχει τα δεδομένα του αισθητήρα σωματιδίων
HM330X air_sensor; // Δημιουργία αντικειμένου για την μέτρηση σωματιδίων
byte SensorPayload[SensorPayloadBufferSize];

void gas_preheat () { // Συνάρτηση για την προθέρμανση του αισθητήρα αερίων
  for (int i = 60 * PRE_HEAT_TIME; i >= 0; i--)
  {
    Serial.print(i / 60);
    Serial.print(":");
    Serial.println(i % 60);
    delay(1000);
    Serial.println("Η προθέρμανση του αισθητήρα ολοκληρώθηκε");
  }
}

float gas_co () { // Συνάρτηση ανάγνωσης του μονοξειδίου του άνθρακα
  if (gas.measure_CO() > 0)
    return gas.measure_CO();
  else
    Serial.println (F("Δεν ήταν δυνατή η μέτρηση του CO"));
}

float gas_no2 () { // Συνάρτηση ανάγνωσης του δυοξειδίου του αζώτου
  if (gas.measure_NO2() > 0)
    return gas.measure_NO2();
  else
    Serial.println (F("Δεν ήταν δυνατή η μέτρηση του ΝΟ2"));
}

float gas_ch4 () { // Συνάρτηση ανάγνωσης του μεθανίου
  if (gas.measure_CH4() > 0)
    return gas.measure_CH4();
  else
    Serial.println (F("Δεν ήταν δυνατή η μέτρηση του CH4(Μεθάνιο)"));
}

float gas_nh3 () { // Συνάρτηση ανάγνωσης της αμμωνίας
  if (gas.measure_NH3() > 0)
    return gas.measure_NH3();
  else
    Serial.println (F("Δεν ήταν δυνατή η μέτρηση της NH3(Αμμωνία)"));
}

float gas_c3h8 () { // Συνάρτηση ανάγνωσης του προπανίου
  if (gas.measure_C3H8() > 0)
    return gas.measure_C3H8();
  else
    Serial.println (F("Δεν ήταν δυνατή η μέτρηση του C3H8(Προπάνιο)"));
}

float bme680temperature () {
  static int32_t  temp, humidity, pressure, gas;
  BME680.getSensorData(temp,humidity,pressure,gas);
  
  return (int8_t)(temp/100);   
}

float bme680humidity () {
  static int32_t  temp, humidity, pressure, gas;
  BME680.getSensorData(temp,humidity,pressure,gas);
  
  return (int16_t)(humidity/1000);   
}

float bme680pressure () {
  static int32_t  temp, humidity, pressure, gas;
  BME680.getSensorData(temp,humidity,pressure,gas);
  
  return (int8_t)(pressure % 100);   
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Προσπάθεια σύνδεσης στο διακομιστή MQTT...");

    // Προσπάθεια σύνδεσης στον MQTT server
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println(" Συνδέθηκε");
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" προσπάθεια σύνδεσης σε 5 δευτερόλεπτα");
      delay(5000);
    }
  }
}

void mqttPublish(char *topic, float payload) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic, String(payload).c_str(), true);
}

void setup() {
  Serial.begin(115200);   // Ενεργοποίηση της σειριακής κονσόλας
    
  Ethernet.begin(mac, ip, myDns); // Αρχικοποίηση της σύνδεσης στο διαδίκτυο με στατική ΙΡ
  Serial.print("Η διεύθυνση ΙΡ του Arduino είναι ");
  Serial.println(Ethernet.localIP());    
  delay(5000); // Χρόνος για την εκκίνηση του Ethernet Shield

  mqttClient.setServer(MQTT_SERVER, 1883);

  while (!BME680.begin(I2C_STANDARD_MODE)) // Start BME680 using I2C, use first device found
  {
    Serial.print(F("-  Unable to find BME680. Trying again in 5 seconds.\n"));
    delay(5000);
  }
  Serial.print(F("- Setting 16x oversampling for all sensors\n"));
  BME680.setOversampling(TemperatureSensor,Oversample16); // Use enumerated type values
  BME680.setOversampling(HumiditySensor,   Oversample16); // Use enumerated type values
  BME680.setOversampling(PressureSensor,   Oversample16); // Use enumerated type values
  Serial.print(F("- Setting IIR filter to a value of 4 samples\n"));
  BME680.setIIRFilter(IIR4); // Use enumerated type values

  gas.begin(GAS_SENSOR); // Ενεργοποίηση του αισθητήρα αερίων, με διεύθυνση στο δίαυλο Ι2C 0x04
  gas.powerOn(); 
  //Serial.println ("Βαθμονόμηση του αισθητήρα αερίων");
  //gas.doCalibrate();

  if (air_sensor.init()) { // Ενεργοποίηση του αισθητήρα σωματιδίων
    Serial.println(F("Απέτυχε η ενεργοποίηση του αισθητήρα σωματιδίων!"));
    while (1);
  }

  if (!mqttClient.connected()) {
    mqttReconnect();
  }

  Serial.println ("Χρόνος για να στεθεροποιηθεί το σύστημα");
  delay(SETUP_TIME);
  Serial.println ("Η αρχική ρύθμιση ολοκληρώθηκε με επιτυχία");
  measure();
}

void loop() {
    if (!mqttClient.connected()) {
      mqttReconnect();
    }
    if (millis() - time_now > TIME_INTERVAL) {
      time_now = millis();
      measure();
    }
    mqttClient.loop();    
}

void measure(){ // Πραγματοποίηση λήψης των μετρήσεων από τους αισθητήρες
  byte sum=0;
  short pm1_0;
  short pm2_5;
  short pm10_0;  
  
  mqttPublish(MQTT_TOPIC_TEMPERATURE, bme680temperature());
  mqttPublish(MQTT_TOPIC_HUMIDITY, bme680humidity());
  mqttPublish(MQTT_TOPIC_PRESSURE, bme680pressure());
  
  mqttPublish(MQTT_TOPIC_CO, gas_co());
  mqttPublish(MQTT_TOPIC_CH4, gas_ch4());
  mqttPublish(MQTT_TOPIC_NO2, gas_no2());
  mqttPublish(MQTT_TOPIC_NH3, gas_nh3());
  
  if(air_sensor.read_sensor_value(SensorPayload,SensorPayloadBufferSize) == NO_ERROR) {
    for(int i=0;i<SensorPayloadLength;i++)
    {
        sum+=SensorPayload[i];
    }
    if(sum!=SensorPayload[SensorPayloadLength])
    {
        Serial.println("Invalid checksum");
        return;
    } 
    pm1_0 = (u16)SensorPayload[SensorPayloadPM1_0Position]<<8|SensorPayload[SensorPayloadPM1_0Position+1];
    mqttPublish(MQTT_TOPIC_PM1_0, pm1_0);
    pm2_5 = (u16)SensorPayload[SensorPayloadPM2_5Position]<<8|SensorPayload[SensorPayloadPM2_5Position+1];
    mqttPublish(MQTT_TOPIC_PM2_5, pm2_5);
    pm10_0 = (u16)SensorPayload[SensorPayloadPM10_0Position]<<8|SensorPayload[SensorPayloadPM10_0Position+1];
    mqttPublish(MQTT_TOPIC_PM10, pm10_0);   
  }
}
