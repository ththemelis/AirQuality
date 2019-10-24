#include <avr/sleep.h>    // Βιβλιοθήκη για τη μείωση της κατανάλωσης ενέργειας
#include <Ethernet.h>
#include <Wire.h>
#include "SI114X.h"     // https://github.com/Seeed-Studio/Grove_Sunlight_Sensor
#include "MutichannelGasSensor.h" // https://github.com/Seeed-Studio/Mutichannel_Gas_Sensor
#include "Seeed_HM330X.h" // https://github.com/Seeed-Studio/Seeed_PM2_5_sensor_HM3301
#include "DS3232RTC.h"    // Βιβλιοθήκη για το ρολόι πραγματικού χρόνου https://github.com/JChristensen/DS3232RTC
#include "seeed_bme680.h" // Βιβλιοθήκη για τον αισθητήρα BME680 https://github.com/Seeed-Studio/BME680_4_In_1_Sensor_Drv

#define interruptPin 2     // Ορισμός του ακροδέκτη 2 για τον έλεγχο των διακοπών (interrupt)
#define BME_ADDR  uint8_t(0x76)   // Ορισμός της διεύθυνσης I2C του αισθητήρα BME680

volatile time_t isrUTC;         // Η μεταβλητή είναι volatile, γιατί η τιμή της αλλάζει μέσα στη συνάρτηση για την εξυπηρέτηση της διακοπής
const int time_interval = 1;    // Ορισμός των λεπτών μεταξύ των μετρήσεων (μεταξύ των διακοπών)

// Ορισμός παραμέτρων για τον αισθητήρα σωματιδίων
u8 buf[100]; // Αρχικοποίηση της μεταβλητής η οποία θα περιέχει τα δεδομένα του αισθητήρα σωματιδίων
HM330X air_sensor; // Δημιουργία αντικειμένου για την μέτρηση σωματιδίων

Seeed_BME680 bme680(BME_ADDR);    // Δημιουργία του αντικειμένου για τον αισθητήρα BME680

err_t parse_result(u8 *data, u8 pm) // Συνάρτηση ανάγνωσης των δεδομένων του αισθητήρα σωματιδίων
{
  //u16 value=0;
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

void setup() {
  Serial.begin(115200);   // Ενεργοποίηση της σειριακής κονσόλας
  
  pinMode(interruptPin,INPUT_PULLUP);
  // Αρχικοποίηση των alarm του RTC
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  time_t t = getUTC();    // Τοποθέτηση της τρέχουσας ώρας στην μεταβλητή t
  t=RTC.get();
  setUTC(t);
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);  // Ορισμός του ALARM1 για ενεργοποίηση μετά από το διάστημα που ορίζει η μεταβλητή time_interval
  RTC.alarm(ALARM_1);
  RTC.squareWave(SQWAVE_NONE);    // Ενεργοποίηση των διακοπών/Απενεργοποίηση της τετραγωνικής κυματομορφής
  RTC.alarmInterrupt(ALARM_1, true);      // Ενεργοποίηση των διακοπών για το ALARM1

  while (!bme680.init()) {    // Ενεργοποίηση του αισθητήρα BME680
    Serial.println("bme680 init failed ! can't find device!");
    delay(10000);
  }

  if (air_sensor.init()) { // Ενεργοποίηση του αισθητήρα σωματιδίων
    Serial.println(F("Απέτυχε η ενεργοποίηση του αισθητήρα σωματιδίων!"));
    while (1);
  }  
}

void loop() {
    delay(100);
    Going_To_Sleep();
}

void Going_To_Sleep(){
    sleep_enable();     // Λειτουργία μείωσης κατανάλωσης
    attachInterrupt(digitalPinToInterrupt(interruptPin), wakeUp, LOW);    // Ορισμός παραμέτρων της διακοπής
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // Ορισμός της μορφής της λειτουργίας για τη μείωση της κατανάλωσης ενέργειας - Ελάχιστη κατανάλωση

    time_t t;
    t=RTC.get();
    Serial.println("Sleep Time: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));    
    delay(10);
    sleep_cpu();    // Ενεργοποίηση της λειτουργίας για τη μείωση της κατανάλωσης ενέργειας

    bme_readings();   // Μετρήσεις από τον αισθητήρα BME680
    Serial.println ("Pm 1.0 "+String(pm25_measurement(5)));
    Serial.println ("Pm 2.5 "+String(pm25_measurement(6)));
    Serial.println ("Pm 10.0 "+String(pm25_measurement(7)));
    t=RTC.get();
    Serial.println("WakeUp Time: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));
    RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);        // Ενεργοποίηση του ALARM1
    RTC.alarm(ALARM_1);
}

void wakeUp(){
    sleep_disable();    // Απενεργοποίηση της λειτουργίας μείωσης κατανάλωσης ενέργειας
    detachInterrupt(digitalPinToInterrupt(interruptPin)); // Απενεργοποίηση των διακοπών από τον ακροδέκτη
}

void bme_readings () {
    if (bme680.read_sensor_data()) {
      Serial.println("Failed to perform reading :(");
      return;
    }
    Serial.print("temperature ===>> ");
    Serial.print(bme680.sensor_result_value.temperature);
    Serial.println(" C");
  
    Serial.print("pressure ===>> ");
    Serial.print(bme680.sensor_result_value.pressure/ 1000.0);
    Serial.println(" KPa");
  
    Serial.print("humidity ===>> ");
    Serial.print(bme680.sensor_result_value.humidity);
    Serial.println(" %"); 
}

time_t getUTC() {   // get the current time
    noInterrupts();
    time_t utc = isrUTC;
    interrupts();
    return utc;
}

void setUTC(time_t utc) { // set the current time
    noInterrupts();
    isrUTC = utc;
    interrupts();
}
