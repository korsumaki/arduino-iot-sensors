#include <Arduino.h>
#include <scheduler.h>
#include <temperature_sensor.h>
#include <humidity_sensor.h>
#include <voltage.h>
#include <content.h> // Screen content handling
#include <value_analysis.h>


#define GATEWAY_WIFI_MQTT
//#define GATEWAY_MESHTASTIC
//#define GATEWAY_MESHTASTIC_TO_WIFI


#ifdef GATEWAY_MESHTASTIC
#include <node.h>

#define MESHTASTIC_SENDING_CHANNEL_INDEX 1

#endif // GATEWAY_MESHTASTIC


#ifdef GATEWAY_WIFI_MQTT

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "YOUR_WIFI_SSID"
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883 // 8883
#define AIO_USERNAME    "YOUR_ADAFRUIT_IO_USERNAME"
#define AIO_KEY         "YOUR_ADAFRUIT_IO_KEY"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Create a unique Client ID using the ESP8266's built-in chip ID
String clientId = "wemos_" + String(ESP.getChipId(), HEX);

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, clientId.c_str(), AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Subscribe timefeed = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");

Adafruit_MQTT_Publish viesti = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/viesti");
Adafruit_MQTT_Publish mqtt_feeds_temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish mqtt_feeds_humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish mqtt_feeds_pressure = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pressure");
Adafruit_MQTT_Publish mqtt_feeds_measurements = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/measurements");

/*************************** Error Reporting *********************************/

Adafruit_MQTT_Subscribe errors = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe throttle = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");

#endif // GATEWAY_WIFI_MQTT


// Display: ESP8266 and ESP32 OLED driver for SSD1306 displays 
// 2x I2C Button (customizable I2C Address, default:0x31)
#include <LOLIN_I2C_BUTTON.h>
#include "SSD1306Wire.h"

#define PRODUCTION_SCHEDULING // Undef for development


#ifdef PRODUCTION_SCHEDULING

#define MEASUREMENT_SEQUENCE_INTERVAL_ms (10*60*1000) // production
#define ANALYZER_TIME_PERIOD_sec (60*60)
#define ANALYZER_TIME_PERIOD_str "h" // Used for output string generation, "min" or "h"

#else // Development

#define MEASUREMENT_SEQUENCE_INTERVAL_ms (6*1000) // development
#define ANALYZER_TIME_PERIOD_sec 60
#define ANALYZER_TIME_PERIOD_str "min" // Used for output string generation, "min" or "h"

#endif // PRODUCTION_SCHEDULING


// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48);

I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS);

bool is_display_available = false;

void init_display(void)
{
    // Init display
    is_display_available = display.init();
    if (is_display_available)
    {
        display.flipScreenVertically();
        display.setContrast(255);

        display.println("IoT mesh\n");
    }
}

uint8_t content_page_index = 0;
uint8_t content_scroll_index = 0;

void button_a_pressed(void)
{
    content_page_index++;
    content_scroll_index = 0;
}

void button_b_pressed(void)
{
    content_scroll_index++;
}

static int display_loop_task(void)
{
    if (is_display_available)
    {
        display.cls();
        if (content_page_index >= content_get_page_count())
        {
            content_page_index = 0;
        }
        display.printf("%s", content_get_page(content_page_index));

        return 500;
    }
    else
    {
        return SCHEDULER_STOP_TASK;
    }
}

void handle_buttons(void)
{
    if (button.get() == 0)
    {
        if (button.BUTTON_A == KEY_VALUE_SHORT_PRESS)
        {
            Serial.println("button A");
            button_a_pressed();
        }

        if (button.BUTTON_B == KEY_VALUE_SHORT_PRESS)
        {
            Serial.println("button B");
            button_b_pressed();
        }
    }
}


const char * content_temperature(void)
{
    static char screen_content_buffer[100] = { 0 };
    sprintf(screen_content_buffer, "Temperature:\n%.2f°C ", temperature_get());
    return screen_content_buffer;
}
const char * content_humidity(void)
{
    float pressure;
    float temp;
    float humidity;
    humidity_sensor_get(pressure, temp, humidity);

    static char screen_content_buffer[100] = { 0 };
    sprintf(screen_content_buffer, "%.2f%%\n%0.2f hPa\n%0.2f°C ", humidity, pressure, temp);
    return screen_content_buffer;
}

const char * content_voltage(void)
{
    static char screen_content_buffer[100] = { 0 };

    float supply_voltage = voltage_get();
    sprintf(screen_content_buffer, "Voltage:\n%0.2f V\n%d, %0.3f ", supply_voltage, adc_value, adc_voltage);
    //sprintf(screen_content_buffer, "Voltage:\n%0.2f V ", supply_voltage);
    return screen_content_buffer;
}

#ifdef GATEWAY_MESHTASTIC

bool node_init_called = false;

const char * content_node(void)
{
    static char screen_content_buffer[100] = { 0 };
    const char * node_state = "not started";
    if (node_init_called)
    {
        if (node_is_connected())
        {
            node_state = "connected";
        }
        else
        {
            node_state = "init";
        }
    }
    sprintf(screen_content_buffer, "Node %s\nsent: %d\nrecv: %d ",
        node_state,
        node_get_sent_count(),
        node_get_received_count());
    return screen_content_buffer;
}

#endif // GATEWAY_MESHTASTIC

const char * content_interval(void)
{
    static char screen_content_buffer[100] = { 0 };
    uint8_t hours = 0;    
    uint8_t minutes = 0;
    uint32_t seconds = MEASUREMENT_SEQUENCE_INTERVAL_ms/1000;

    // Divide interval to hours, minutes and seconds
    hours = seconds / (60*60);
    seconds -= hours * (60*60);

    minutes = seconds / 60;
    seconds -= minutes * 60;

    int index = 0;
    index += sprintf(&screen_content_buffer[index], "Interval:\n");

    if (hours > 0)
    {
        index += sprintf(&screen_content_buffer[index], "%dh ", hours);
    }
    if (minutes > 0)
    {
        index += sprintf(&screen_content_buffer[index], "%dmin ", minutes);
    }
    if (seconds > 0)
    {
        index += sprintf(&screen_content_buffer[index], "%dsec ", seconds);
    }

    return screen_content_buffer;
}

#ifdef GATEWAY_WIFI_MQTT

const char * content_wifi(void)
{
    static char screen_content_buffer[100] = { 0 };
    wl_status_t status = WiFi.status();
    static const char* status_str[] =
    {
        "IDLE_STATUS",
        "NO_SSID_AVAIL",
        "SCAN_COMPLETED",
        "CONNECTED",
        "CONNECT_FAILED",
        "CONNECTION_LOST",
        "WRONG_PASSWORD",
        "DISCONNECTED"
    };

    sprintf(screen_content_buffer, "WiFi:\n%s\n%s ",
        WiFi.SSID().c_str(),
        status_str[status]);

    return screen_content_buffer;
}

const char * content_mqtt(void)
{
    static char screen_content_buffer[100] = { 0 };
    const char * status = "Disconnected";
    if (mqtt.connected())
    {
        status = "Connected";
    }
    sprintf(screen_content_buffer, "MQTT %s\n%s\n%s ", status, AIO_SERVER, clientId.c_str());

    return screen_content_buffer;
}

#endif // GATEWAY_WIFI_MQTT


value_analysis_t temperature_analyzer;
value_analysis_t pressure_analyzer;
value_analysis_t temp_analyzer;
value_analysis_t humidity_analyzer;
value_analysis_t voltage_analyzer;

static void init_analyzers(void)
{
    value_analysis_init(&temperature_analyzer, ANALYZER_TIME_PERIOD_sec);
    value_analysis_init(&pressure_analyzer, ANALYZER_TIME_PERIOD_sec);
    value_analysis_init(&temp_analyzer, ANALYZER_TIME_PERIOD_sec);
    value_analysis_init(&humidity_analyzer, ANALYZER_TIME_PERIOD_sec);
    value_analysis_init(&voltage_analyzer, ANALYZER_TIME_PERIOD_sec);
}


static int measurement_analysis_task(void)
{
    char screen_content_buffer[200] = { 0 };
    static int msg_counter = 0;

    // ==================================
    // get values
    float temperature = temperature_get();

    float pressure;
    float temp;
    float humidity;
    humidity_sensor_get(pressure, temp, humidity);

    float supply_voltage = voltage_get();

    // ==================================
    // do analysis
    long current_time = millis();

    float temperature_change = value_analysis_add_value(&temperature_analyzer, temperature, current_time);
    float pressure_change = value_analysis_add_value(&pressure_analyzer, pressure, current_time);
    float temp_change = value_analysis_add_value(&temp_analyzer, temp, current_time);
    float humidity_change = value_analysis_add_value(&humidity_analyzer, humidity, current_time);
    float voltage_change = value_analysis_add_value(&voltage_analyzer, supply_voltage, current_time);

    // ==================================
    // create string
    int index = 0;

    msg_counter++;
    index += sprintf(&screen_content_buffer[index], "#%d\n", msg_counter);

    if (temperature != -127)
    {
        index += sprintf(&screen_content_buffer[index], "DS18B20: Temperature %.2f°C (%.2f°C/%s)\n", temperature, temperature_change, ANALYZER_TIME_PERIOD_str);
    }
    else
    {
        index += sprintf(&screen_content_buffer[index], "DS18B20: Not connected\n");
    }

    if (pressure != NAN)
    {
        index += sprintf(&screen_content_buffer[index], "BME280: Pressure %.2f hPa (%.2f hPa/%s), Humidity %.2f%% (%.2f%%/%s), Temperature %.2f°C (%.2f°C/%s)\n",
            pressure, pressure_change, ANALYZER_TIME_PERIOD_str,
            humidity, humidity_change, ANALYZER_TIME_PERIOD_str,
            temp, temp_change, ANALYZER_TIME_PERIOD_str);
    }
    else
    {
        index += sprintf(&screen_content_buffer[index], "BME280: Not connected\n");
    }

    index += sprintf(&screen_content_buffer[index], "Voltage: %.2fV (%.2fV/%s) ", supply_voltage, voltage_change, ANALYZER_TIME_PERIOD_str);

    // ==================================
    // send to meshtastic
    Serial.println("==================================");
    Serial.println(screen_content_buffer);
    Serial.println("==================================");

#ifdef GATEWAY_MESHTASTIC
    node_send_message(MESHTASTIC_SENDING_CHANNEL_INDEX, screen_content_buffer);
#endif // GATEWAY_MESHTASTIC

#ifdef GATEWAY_WIFI_MQTT
    // Send mqtt messages
    mqtt_feeds_measurements.publish(screen_content_buffer);
    mqtt_feeds_temperature.publish(temp);
    mqtt_feeds_humidity.publish(humidity);
    mqtt_feeds_pressure.publish(pressure);
#endif // GATEWAY_WIFI_MQTT

    return SCHEDULER_STOP_TASK;
}


static int measure_sequence_task(void)
{
    voltage_measure();
    temperature_sensor_measure();
    humidity_sensor_measure();

    // Do measurement analysis after 2 seconds. All measurements should be ready then.
    scheduler_add_task(measurement_analysis_task, 2000);
    return MEASUREMENT_SEQUENCE_INTERVAL_ms;
}

#ifdef GATEWAY_MESHTASTIC

#define NODE_REINIT_TIME_IF_NOT_CONNECTED (30*1000)

static int node_start_task(void)
{
    static uint8_t state = 0;
    static long init_called_at_time = 0;
    int time_for_next_call = SCHEDULER_STOP_TASK;
    switch (state)
    {
    case 0: // from start, wait >10 sec before node_init()
        state = 1;
        time_for_next_call = 12*1000;
        break;
    case 1: // node_init()
        state = 2;
        node_init();
        init_called_at_time = millis();
        node_init_called = true;
        time_for_next_call = 1;
        break;
    case 2: // node_loop()
        node_loop();
        time_for_next_call = 2;

        if (!node_is_connected() && (millis() - init_called_at_time) > NODE_REINIT_TIME_IF_NOT_CONNECTED )
        {
            state = 1; // Go back to init state if not connected
        }
        break;
    default:
        break;
    }
    return time_for_next_call;
}

#endif // GATEWAY_MESHTASTIC

#ifdef GATEWAY_WIFI_MQTT

void mqtt_setup(void);
void mqtt_loop(uint32_t usable_time_ms);

#endif // GATEWAY_WIFI_MQTT

void setup()
{
    Serial.begin(115200);
    scheduler_init(millis);

    init_display();
    content_init();

#ifdef GATEWAY_WIFI_MQTT
    content_add_page(content_wifi);
    content_add_page(content_mqtt);
#endif // GATEWAY_WIFI_MQTT

#ifdef GATEWAY_MESHTASTIC
    content_add_page(content_node);
#endif // GATEWAY_MESHTASTIC

    content_add_page(content_temperature);
    content_add_page(content_humidity);
    content_add_page(content_voltage);
    content_add_page(content_interval);

    init_analyzers();

    temperature_sensor_init();
    humidity_sensor_init();

    // Add task for display updating
    scheduler_add_task(display_loop_task, 0);
    scheduler_add_task(measure_sequence_task, 1000*3);

#ifdef GATEWAY_MESHTASTIC
    scheduler_add_task(node_start_task, 0);
#endif // GATEWAY_MESHTASTIC

#ifdef GATEWAY_WIFI_MQTT
    // MQTT setup
    mqtt_setup();
#endif // GATEWAY_WIFI_MQTT
}

void loop()
{
    handle_buttons();
    uint32_t next_run_ms = scheduler_loop();
    (void)next_run_ms;
#ifdef GATEWAY_WIFI_MQTT
    mqtt_loop(next_run_ms);
#endif // GATEWAY_WIFI_MQTT
}


#ifdef GATEWAY_WIFI_MQTT
// MQTT stuff

void MQTT_connect(void);


void timecallback(uint32_t current)
{
    Serial.printf("time is %d\n", current);
}

void error_callback(char *str, uint16_t len)
{
    Serial.printf("error_callback: %s\n", str);
}

void throttle_callback(char *str, uint16_t len)
{
    Serial.printf("throttle_callback: %s\n", str);
}

void mqtt_setup(void)
{
    Serial.println(F("Initialize WiFi and mqtt"));
    Serial.printf("clientId: %s\n", clientId.c_str());

    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(F("."));
    }
    Serial.println(F(" WiFi connected."));

    //timefeed.setCallback(timecallback);
    //mqtt.subscribe(&timefeed);

    // Setup MQTT subscriptions for throttle & error messages
    errors.setCallback(error_callback);
    mqtt.subscribe(&errors);
    throttle.setCallback(throttle_callback);
    mqtt.subscribe(&throttle);
}

void mqtt_loop(uint32_t usable_time_ms)
{
    // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.
    MQTT_connect();

    static bool started = false;
    if (!started)
    {
        started = true;
        if (!viesti.publish("Running!"))
        {
            Serial.println(F("publish: Failed"));
        }
    }

    // wait for subscription messages
    mqtt.processPackets(usable_time_ms);

    // keep the connection alive
    mqtt.ping();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect(void)
{
    // Stop if already connected.
    if (mqtt.connected())
    {
        return;
    }

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 3;
    int8_t ret;
    while ((ret = mqtt.connect()) != 0)
    { // connect will return 0 for connected
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000); // wait 5 seconds
        retries--;
        if (retries == 0)
        {
            // basically die and wait for WDT to reset me
            while (1)
                ;
        }
    }

    Serial.println("MQTT Connected!");
}
#endif // GATEWAY_WIFI_MQTT
