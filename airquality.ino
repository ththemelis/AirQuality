#include <avr/sleep.h>    //this AVR library contains the methods that controls the sleep modes
#include <Wire.h>
#include "DS3232RTC.h"    // https://github.com/JChristensen/DS3232RTC
#include "seeed_bme680.h"

#define interruptPin 2     // Ορισμός του ακροδέκτη 2 για τον έλεγχο των διακοπών (interrupt)

volatile time_t isrUTC;         // ISR's copy of current time in UTC

#define BME_ADDR  uint8_t(0x76)

Seeed_BME680 bme680(BME_ADDR);

const int time_interval = 1;    // Ορισμός των λεπτών μεταξύ των μετρήσεων (μεταξύ των διακοπών)

void setup() {
  Serial.begin(115200);   // Ενεργοποίηση της σειριακής κονσόλας
  
  pinMode(interruptPin,INPUT_PULLUP); //Set pin d2 to input using the buildin pullup resistor
  // Αρχικοποίηση των alarm 
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  time_t t = getUTC();    // Τοποθέτηση της τρέχουσας ώρας στην μεταβλητή t
  t=RTC.get();    // Παίρνει την τρέχουσα ώρα από το ρολόι (RTC)
  setUTC(t);
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);  // Ορισμός του ALARM1 για ενεργοποίηση μετά από το διάστημα που ορίζει η μεταβλητή time_interval
  RTC.alarm(ALARM_1);   // clear the alarm flag
  RTC.squareWave(SQWAVE_NONE);    // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
  RTC.alarmInterrupt(ALARM_1, true);      // enable interrupt output for Alarm 1

  while (!bme680.init()) 
  {
    Serial.println("bme680 init failed ! can't find device!");
  delay(10000);
  }  
}

void loop() {
  delay(100);//wait 5 seconds before going to sleep. In real senairio keep this as small as posible
  Going_To_Sleep();
}

void Going_To_Sleep(){
    sleep_enable();//Enabling sleep mode
    attachInterrupt(0, wakeUp, LOW);//attaching a interrupt to pin d2
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);//Setting the sleep mode, in our case full sleep
    digitalWrite(LED_BUILTIN,LOW);//turning LED off
    time_t t;// creates temp time variable
    t=RTC.get(); //gets current time from rtc
    Serial.println("Sleep  Time: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));//prints time stamp on serial monitor
    delay(100); //wait a second to allow the led to be turned off before going to sleep
    sleep_cpu();//activating sleep mode

    bme_readings();//function that reads the temp and the humidity
    t=RTC.get();
    Serial.println("WakeUp Time: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));//Prints time stamp 
    //Set New Alarm
    RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);
  
  // clear the alarm flag
  RTC.alarm(ALARM_1);
  }

  void wakeUp(){
  Serial.println("Interrrupt Fired");//Print message to serial monitor
   sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;
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

time_t getUTC()
{
    noInterrupts();
    time_t utc = isrUTC;
    interrupts();
    return utc;
}

// set the current time
void setUTC(time_t utc)
{
    noInterrupts();
    isrUTC = utc;
    interrupts();
}
