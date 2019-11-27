#define BME_ADDR  uint8_t(0x76)   // Ορισμός της διεύθυνσης I2C του αισθητήρα BME680
#define GAS_SENSOR uint8_t(0x04) // H διεύθυνση του αισθητήρα στον διαύλο I2C

#define TIME_INTERVAL 300000

#define MQTT_TOPIC_HUMIDITY "airquality/lab19/humidity"
#define MQTT_TOPIC_TEMPERATURE "airquality/lab19/temperature"
#define MQTT_TOPIC_PRESSURE "airquality/lab19/pressure"
#define MQTT_TOPIC_PM1_0 "airquality/lab19/pm1_0"
#define MQTT_TOPIC_PM2_5 "airquality/lab19/pm2_5"
#define MQTT_TOPIC_PM10 "airquality/lab19/pm10"
#define MQTT_TOPIC_NO2 "airquality/lab19/no2"
#define MQTT_TOPIC_NH3 "airquality/lab19/nh3"
#define MQTT_TOPIC_CO "airquality/lab19/co"
#define MQTT_TOPIC_CH4 "airquality/lab19/ch4"
#define MQTT_TOPIC_STATE "ON"
#define MQTT_CLIENT_ID "arduino19"

const char *MQTT_SERVER = "192.168.1.50";
const char *MQTT_USER = "mqttadmin";
const char *MQTT_PASSWORD = "cvHVf5bPpwLu6tRbf6z9";
