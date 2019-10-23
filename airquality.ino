#include <avr/sleep.h>    // Βιβλιοθήκη για τη μείωση της κατανάλωσης ενέργειας
#include <Wire.h>         
#include "DS3232RTC.h"    // Βιβλιοθήκη για το ρολόι πραγματικού χρόνου https://github.com/JChristensen/DS3232RTC
#include "seeed_bme680.h" // Βιβλιοθήκη για τον αισθητήρα BME680 https://github.com/Seeed-Studio/BME680_4_In_1_Sensor_Drv

#define interruptPin 2     // Ορισμός του ακροδέκτη 2 για τον έλεγχο των διακοπών (interrupt)
#define BME_ADDR  uint8_t(0x76)   // Ορισμός της διεύθυνσης I2C του αισθητήρα BME680

volatile time_t isrUTC;         // Η μεταβλητή είναι volatile, γιατί η τιμή της αλλάζει μέσα στη συνάρτηση για την εξυπηρέτηση της διακοπής
const int time_interval = 1;    // Ορισμός των λεπτών μεταξύ των μετρήσεων (μεταξύ των διακοπών)

Seeed_BME680 bme680(BME_ADDR);    // Δημιουργία του αντικειμένου για τον αισθητήρα BME680

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

  while (!bme680.init())    // Ενεργοποίηση του αισθητήρα BME680
  {
    Serial.println("bme680 init failed ! can't find device!");
    delay(10000);
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
    delay(10);
    sleep_cpu();    // Ενεργοποίηση της λειτουργίας για τη μείωση της κατανάλωσης

    bme_readings();   // Μετρήσεις από τον αισθητήρα BME680
    t=RTC.get();
    RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);        // Ενεργοποίηση του ALARM1
    RTC.alarm(ALARM_1);
}

void wakeUp(){
    sleep_disable();    // Απενεργοποίηση της λειτουργίας μείωσης κατανάλωσης ενέργειας
    detachInterrupt(digitalPinToInterrupt(interruptPin)); // Απενεργοποίηση των διακοπών από τον ακροδέκτη
}

void bme_readings () {
    if (bme680.read_sensor_data()) 
    {
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
