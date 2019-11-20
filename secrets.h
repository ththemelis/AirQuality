#define BME_ADDR  uint8_t(0x76)   // Ορισμός της διεύθυνσης I2C του αισθητήρα BME680
#define GAS_SENSOR uint8_t(0x04) // H διεύθυνση του αισθητήρα στον διαύλο I2C

#define TIME_INTERVAL 300000

#define MQTT_TOPIC_HUMIDITY "airquality/bme680/humidity"
#define MQTT_TOPIC_TEMPERATURE "airquality/bme680/temperature"
#define MQTT_TOPIC_PRESSURE "airquality/bme680/pressure"
#define MQTT_TOPIC_PM1 "airquality/particles/pm1"
#define MQTT_TOPIC_PM2 "airquality/particles/pm2"
#define MQTT_TOPIC_PM3 "airquality/particles/pm3"
#define MQTT_TOPIC_NO2 "airquality/gases/no2"
#define MQTT_TOPIC_NH3 "airquality/gases/nh3"
#define MQTT_TOPIC_CO "airquality/gases/co"
#define MQTT_TOPIC_CH4 "airquality/gases/ch4"
#define MQTT_TOPIC_STATE "airquality/bme680/status"
#define MQTT_CLIENT_ID "arduino19"

const char *MQTT_SERVER = "192.168.1.50";
const char *MQTT_USER = "mqttadmin";
const char *MQTT_PASSWORD = "cvHVf5bPpwLu6tRbf6z9";
