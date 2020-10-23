// Ορισμός παραμέτρων για την ενσύρματη σύνδεση στο διαδίκτυο
byte mac[] = {0x2C, 0xF7, 0xF1, 0x08, 0x27, 0xE0};
IPAddress ip(192, 168, 1, 241); // Η στατική διεύθυνση IP του Ethernet Shield, σε περίπτωση που δεν πάρει διεύθυνση ΙΡ μέσω DHCP
IPAddress myDns(192, 168, 1, 1); // Η διεύθυνση δρομολογητή, σε περίπτωση που δεν πάρει διεύθυνση ΙΡ μέσω DHCP

#define BME_ADDR  uint8_t(0x76)   // Ορισμός της διεύθυνσης I2C του αισθητήρα BME680
#define GAS_SENSOR uint8_t(0x04) // H διεύθυνση του αισθητήρα στον διαύλο I2C
#define PRE_HEAT_TIME   10 // Διάρκεια προθέρμανσης (σε λεπτά). Απαιτούνται τουλάχιστον 10 λεπτά.

#define TIME_INTERVAL 180000 // Χρόνος μεταξύ των μετρήσεων
#define SETUP_TIME 60000  // Αρχικός χρόνος για την σταθεροποίηση του συστήματος (1 λεπτό)

#define MQTT_TOPIC_HUMIDITY "airquality/pektpe/humidity"
#define MQTT_TOPIC_TEMPERATURE "airquality/pektpe/temperature"
#define MQTT_TOPIC_PRESSURE "airquality/pektpe/pressure"
#define MQTT_TOPIC_PM1_0 "airquality/pektpe/pm1_0"
#define MQTT_TOPIC_PM2_5 "airquality/pektpe/pm2_5"
#define MQTT_TOPIC_PM10 "airquality/pektpe/pm10"
#define MQTT_TOPIC_NO2 "airquality/pektpe/no2"
#define MQTT_TOPIC_NH3 "airquality/pektpe/nh3"
#define MQTT_TOPIC_CO "airquality/pektpe/co"
#define MQTT_TOPIC_CH4 "airquality/pektpe/ch4"
#define MQTT_TOPIC_STATE "ON"
#define MQTT_CLIENT_ID "2gymkas"