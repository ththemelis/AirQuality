#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "Seeed_HM330X.h" // https://github.com/Seeed-Studio/Seeed_PM2_5_sensor_HM3301
#include "seeed_bme680.h" // Βιβλιοθήκη για τον αισθητήρα BME680 https://github.com/Seeed-Studio/BME680_4_In_1_Sensor_Drv
#include "MutichannelGasSensor.h" // https://github.com/Seeed-Studio/Mutichannel_Gas_Sensor
#include "secrets.h"

int times=0; // Μεταβλητή που κρατάει το πλήθος των μετρήσεων. Κάθε πέντε μετρήσεις γίνεται υπολογισμός του μέσου όρου και γίνεται αποστολή δεδομένων στον MQTT Broker
float tem[5], hum[5], pres[5]; // Πίνακες που αποθηκευόνται τα δεδομένα των μετρήσεων
float pm1_0[5],pm2_5[5],pm10[5];
float co[5],no2[5],nh3[5],ch4[5];

unsigned long time_now = 0;

EthernetClient ethClient; // Δημιουργία αντικειμένου για την ενσύρματη σύνδεση στο διαδίκτυο

PubSubClient mqttClient(ethClient);

// Ορισμός παραμέτρων λειτουργίας του αισθητήρα θερμοκρασίας/υγρασίας/ατμ. πίεσης
Seeed_BME680 bme680(BME_ADDR);    // Δημιουργία του αντικειμένου για τον αισθητήρα BME680

// Ορισμός παραμέτρων για τον αισθητήρα σωματιδίων
u8 buf[100]; // Αρχικοποίηση της μεταβλητής η οποία θα περιέχει τα δεδομένα του αισθητήρα σωματιδίων
HM330X air_sensor; // Δημιουργία αντικειμένου για την μέτρηση σωματιδίων

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

float temper() { // Συνάρτηση ανάγνωσης της θερμοκρασίας
  if (bme680.read_sensor_data())
  {
    Serial.println(F("Δεν ήταν δυνατή η μέτρηση της θερμοκρασίας"));
    return;
  }
  return bme680.sensor_result_value.temperature;
}

float humidity() { // Συνάρτηση ανάγνωσης της υγρασίας
  if (bme680.read_sensor_data()) {
    Serial.println(F("Δεν ήταν δυνατή η μέτρηση της υγρασίας"));
    return;
  }
  return bme680.sensor_result_value.humidity;
}

float pressure() { // Συνάρτηση ανάγνωσης της ατμοσφαιρικής πίεσης
  if (bme680.read_sensor_data()) {
    Serial.println(F("Δεν ήταν δυνατή η μέτρηση της ατμ. πίεσης"));
    return;
  }
  return bme680.sensor_result_value.pressure / 1000.0;
}

err_t parse_result(u8 *data, u8 pm) { // Συνάρτηση ανάγνωσης των δεδομένων του αισθητήρα σωματιδίων
  err_t NO_ERROR;
  if (NULL == data)
    return ERROR_PARAM;
  return (u16)data[pm * 2] << 8 | data[pm * 2 + 1];
}

float pm25_measurement (int sense) {

  if (air_sensor.read_sensor_value(buf, 29)) {
    Serial.println(F("Δεν ήταν δυνατή η μέτρηση σωματιδίων!"));
  }

  if (sense == 2)
    return parse_result(buf, 2); // Συγκέντρωση σωματιδίων PM1.0 Std
  else if (sense == 3)
    return parse_result(buf, 3); // Συγκέντρωση σωματιδίων PM2.5 Std
  else if (sense == 4)
    return parse_result(buf, 4); // Συγκέντρωση σωματιδίων PM10 Std
  else if (sense == 5)
    return parse_result(buf, 5); // Συγκέντρωση σωματιδίων PM1.0 Atm
  else if (sense == 6)
    return parse_result(buf, 6); // Συγκέντρωση σωματιδίων PM2.5 Atm
  else if (sense == 7)
    return parse_result(buf, 7); // Συγκέντρωση σωματιδίων PM10 Atm
  else if (sense == 8)
    return parse_result(buf, 8); // Σωματίδια μεγαλύτερα από 0.3μm
  else if (sense == 9)
    return parse_result(buf, 9); // Σωματίδια μεγαλύτερα από 0.5μm
  else if (sense == 10)
    return parse_result(buf, 10); // Σωματίδια μεγαλύτερα από 1.0μm
  else if (sense == 11)
    return parse_result(buf, 11); // Σωματίδια μεγαλύτερα από 2.5μm
  else if (sense == 12)
    return parse_result(buf, 12); // Σωματίδια μεγαλύτερα από 5.0μm
  else
    return parse_result(buf, 13); // Σωματίδια μεγαλύτερα από 10.0μm
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Προσπάθεια σύνδεσης στο διακομιστή MQTT...");

    // Προσπάθεια σύνδεσης στον MQTT server
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("Συνδέθηκε");
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

  while (!bme680.init()) {    // Ενεργοποίηση του αισθητήρα BME680
    Serial.println("Απέτυχε η ενεργοποίηση του αισθητήρα θερμοκρασίας/υγρασίας!");
    while (1);
  }

  gas.begin(GAS_SENSOR); // Ενεργοποίηση του αισθητήρα αερίων, με διεύθυνση στο δίαυλο Ι2C 0x04
  unsigned char version = gas.getVersion();
  Serial.print("Multichannel Gas Sensor version = ");
  Serial.println(version);  
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

  print ("Χρόνος για να στεθεροποιηθεί το σύστημα")
  delay(SETUP_TIME);
  measure();
}

float avg (float *arr, int len) { // Υπολογισμός του μέσου όρου των πέντε τελευταίων μετρήσεων
  float sum = 0L;
  for (int i = 0; i < len; i++) {
    sum += arr[i];
  }
  return sum / len;
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
  tem[times] = temper();
  hum[times] = humidity();
  pres[times] = pressure();
  pm1_0[times] = pm25_measurement(5);
  pm2_5[times] = pm25_measurement(6);
  pm10[times] = pm25_measurement(7);
  co[times] = gas_co();
  ch4[times] = gas_ch4();
  no2[times] = gas_no2();
  nh3[times] = gas_nh3();

  times = times + 1;
  if (times == 5) { // Κάθε πέντε μετρήσεις υπολογίζεται ο μέσος όρος και γίνεται αποστολή στον MQTT Broker
    mqttPublish(MQTT_TOPIC_TEMPERATURE, avg(tem,times));
    mqttPublish(MQTT_TOPIC_HUMIDITY, avg(hum,times));
    mqttPublish(MQTT_TOPIC_PRESSURE, avg(pres,times));
    mqttPublish(MQTT_TOPIC_CO, avg(co,times));
    mqttPublish(MQTT_TOPIC_CH4, avg(ch4,times));
    mqttPublish(MQTT_TOPIC_NO2, avg(no2,times));
    mqttPublish(MQTT_TOPIC_NH3, avg(nh3,times));
    mqttPublish(MQTT_TOPIC_PM1_0, avg(pm1_0,times));
    mqttPublish(MQTT_TOPIC_PM2_5, avg(pm2_5,times));
    mqttPublish(MQTT_TOPIC_PM10, avg(pm10,times));
    times = 0;
  }
}
