#include <avr/sleep.h>    // Βιβλιοθήκη για τη μείωση της κατανάλωσης ενέργειας
#include <avr/power.h>
#include <Ethernet.h>
#include <SPI.h>
//#include <Wire.h>
#include <PubSubClient.h>
#include "Seeed_HM330X.h" // https://github.com/Seeed-Studio/Seeed_PM2_5_sensor_HM3301
#include <DS3232RTC.h>    // Βιβλιοθήκη για το ρολόι πραγματικού χρόνου https://github.com/JChristensen/DS3232RTC https://github.com/PaulStoffregen/Time
#include "seeed_bme680.h" // Βιβλιοθήκη για τον αισθητήρα BME680 https://github.com/Seeed-Studio/BME680_4_In_1_Sensor_Drv
#include "MutichannelGasSensor.h" // https://github.com/Seeed-Studio/Mutichannel_Gas_Sensor
#include "secrets.h"

// Ορισμός παραμέτρων για την ενσύρματη σύνδεση στο διαδίκτυο
byte mac[] = {0x2C, 0xF7, 0xF1, 0x08, 0x27, 0xE0}; // Η διεύθυνση MAC του Ethernet Shield
IPAddress ip(192, 168, 1, 49); // Η στατική διεύθυνση IP του Ethernet Shield, σε περίπτωση που δεν πάρει διεύθυνση ΙΡ μέσω DHCP
IPAddress myDns(192, 168, 1, 1); // Η διεύθυνση δρομολογητή, σε περίπτωση που δεν πάρει διεύθυνση ΙΡ μέσω DHCP
EthernetClient ethClient; // Δημιουργία αντικειμένου για την ενσύρματη σύνδεση στο διαδίκτυο

// Ορισμός παραμέτρων για τον MQTT broker
PubSubClient mqttClient(ethClient);

// Ορισμός παραμέτρων λειτουργίας του αισθητήρα αερίων
#define PRE_HEAT_TIME   10 // Διάρκεια προθέρμανσης (σε λεπτά). Απαιτούνται τουλάχιστον 10 λεπτά.

// Ορισμός παραμέτρων λειτουργίας του αισθητήρα θερμοκρασίας/υγρασίας/ατμ. πίεσης
Seeed_BME680 bme680(BME_ADDR);    // Δημιουργία του αντικειμένου για τον αισθητήρα BME680

// Ορισμός παραμέτρων διακοπών
//#define interruptPin 3     // Ορισμός του ακροδέκτη 2 για τον έλεγχο των διακοπών (interrupt)
//volatile time_t isrUTC;         // Η μεταβλητή είναι volatile, γιατί η τιμή της αλλάζει μέσα στη συνάρτηση για την εξυπηρέτηση της διακοπής
//const int time_interval = 5;    // Ορισμός των λεπτών μεταξύ των μετρήσεων (μεταξύ των διακοπών)

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
  return bme680.sensor_result_value.pressure;
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
      Serial.println(" προσπάθεια σύνδεσης σε 5 δευτερλόπετα");
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

time_t compileTime() {
    const time_t FUDGE(10);    //fudge factor to allow for upload time, etc. (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char compMon[3], *m;

    strncpy(compMon, compDate, 3);
    compMon[3] = '\0';
    m = strstr(months, compMon);

    tmElements_t tm;
    tm.Month = ((m - months) / 3 + 1);
    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);

    time_t t = makeTime(tm);
    return t + FUDGE;        //add fudge factor to allow for compile time
}

time_t getUTC() {   // Διάβασμα από το RTC της ώρας
    noInterrupts();
    time_t utc = isrUTC;
    interrupts();
    return utc;
}

void setUTC(time_t utc) { // Εγγραφή της ώρας στο RTC
    noInterrupts();
    isrUTC = utc;
    interrupts();
}

void setup() {
  Serial.begin(115200);   // Ενεργοποίηση της σειριακής κονσόλας
  
  //pinMode(interruptPin,INPUT_PULLUP);
  // Αρχικοποίηση των alarm του RTC
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  RTC.set(compileTime());

  //time_t t = getUTC();    // Τοποθέτηση της τρέχουσας ώρας στην μεταβλητή t
  //t=RTC.get();
  //setUTC(t);
  RTC.setAlarm(ALM1_MATCH_SECONDS, 0, 0, 0, time_interval);  // Ορισμός του ALARM1 για ενεργοποίηση μετά από το διάστημα που ορίζει η μεταβλητή time_interval
  RTC.alarm(ALARM_1);
  RTC.squareWave(SQWAVE_NONE);    // Ενεργοποίηση των διακοπών/Απενεργοποίηση της τετραγωνικής κυματομορφής
  //RTC.alarmInterrupt(ALARM_1, true);      // Ενεργοποίηση των διακοπών για το ALARM1

  Ethernet.begin(mac, ip, myDns); // Αρχικοποίηση της σύνδεσης στο διαδίκτυο με στατική ΙΡ
  Serial.print("Η διεύθυνση ΙΡ του Arduino είναι ");
  Serial.println(Ethernet.localIP());    
  delay(5000); // Χρόνος για την εκκίνηση του Ethernet Shield

  mqttClient.setServer(MQTT_SERVER, 1883);

  while (!bme680.init()) {    // Ενεργοποίηση του αισθητήρα BME680
    Serial.println("Πρόβλημα στον αισθητήρα bme680!");
    delay(10000);
  }

  gas.begin(GAS_SENSOR); // Ενεργοποίηση του αισθητήρα αερίων, με διεύθυνση στο δίαυλο Ι2C 0x04
  gas.powerOn();
  Serial.print("Firmware ");
  Serial.println(gas.getVersion());  
  //Serial.println ("Βαθμονόμηση του αισθητήρα αερίων");
  //gas.doCalibrate();

  if (air_sensor.init()) { // Ενεργοποίηση του αισθητήρα σωματιδίων
    Serial.println(F("Απέτυχε η ενεργοποίηση του αισθητήρα σωματιδίων!"));
    while (1);
  }  
}

void loop() {
    if (RTC.alarm(ALARM_1)) {
      measure();
    }  
    if (!mqttClient.connected()) {
      mqttReconnect();
    }
    mqttClient.loop();    
}

void measure(){
//    time_t t;
//    t=RTC.get();
//    Serial.println("Sleep Time: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));    
//    delay(10);

//    float tem=temper();
//    float hum=humidity();
//   float pre=pressure();
//    float co=gas_co();
//    float no2=gas_no2();
//    float pm1=pm25_measurement(5);
//    float pm2=pm25_measurement(6);
//    float pm3=pm25_measurement(7);
    
    /*Serial.println ("Temperature:"+String(tem));
    Serial.println ("Humidity:"+String(hum));
    Serial.println ("Pressure:"+String(pre));
    Serial.println ("Pm 1.0 "+String(pm1));
    Serial.println ("Pm 2.5 "+String(pm2));
    Serial.println ("Pm 10.0 "+String(pm3));
    Serial.println ("CO:"+String(co));
    Serial.println ("NO2:"+String(no2));*/

    mqttPublish(MQTT_TOPIC_TEMPERATURE, temper());
    mqttPublish(MQTT_TOPIC_HUMIDITY, humidity());
    mqttPublish(MQTT_TOPIC_PRESSURE, pre);
    mqttPublish(MQTT_TOPIC_CO, gas_co());
    mqttPublish(MQTT_TOPIC_NOX, gas_no2());
    mqttPublish(MQTT_TOPIC_PM1, pm25_measurement(5));
    mqttPublish(MQTT_TOPIC_PM2, pm25_measurement(6));
    mqttPublish(MQTT_TOPIC_PM3, pm25_measurement(7));

//    t=RTC.get();
//    Serial.println("WakeUp Time: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));
//    RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);        // Ενεργοποίηση του ALARM1
//    RTC.alarm(ALARM_1);
}
