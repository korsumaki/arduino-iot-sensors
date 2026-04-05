#include <Arduino.h>
#include <scheduler.h>
#include <temperature_sensor.h>
#include <humidity_sensor.h>
#include <voltage.h>
#include <content.h> // Screen content handling
#include <node.h>


// Display: ESP8266 and ESP32 OLED driver for SSD1306 displays 
// 2x I2C Button (customizable I2C Address, default:0x31)
#include <LOLIN_I2C_BUTTON.h>
#include "SSD1306Wire.h"


//#define MEASUREMENT_SEQUENCE_INTERVAL_ms (5*1000) // development
#define MEASUREMENT_SEQUENCE_INTERVAL_ms (10*60*1000) // production


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

static int measurement_analysis_task(void)
{
    char screen_content_buffer[150] = { 0 };
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

    // ==================================
    // create string
    int index = 0;

    msg_counter++;
    index += sprintf(&screen_content_buffer[index], "#%d\n", msg_counter);

    if (temperature != -127)
    {
        index += sprintf(&screen_content_buffer[index], "DS18B20: Temperature %.2f°C\n", temperature);
    }
    else
    {
        index += sprintf(&screen_content_buffer[index], "DS18B20: Not connected\n");
    }

    if (pressure != NAN)
    {
        index += sprintf(&screen_content_buffer[index], "BME280: Pressure %.2f hPa, Humidity %.2f%%, Temperature %.2f°C\n",
            pressure, humidity, temp);
    }
    else
    {
        index += sprintf(&screen_content_buffer[index], "BME280: Not connected\n");
    }

    index += sprintf(&screen_content_buffer[index], "Voltage: %.2fV ", supply_voltage);

    // ==================================
    // send to meshtastic
    Serial.println("==================================");
    Serial.println(screen_content_buffer);
    Serial.println("==================================");

    node_send_message(1, screen_content_buffer);

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


void setup()
{
    Serial.begin(115200);
    scheduler_init(millis);

    init_display();
    content_init();
    content_add_page(content_node);
    content_add_page(content_temperature);
    content_add_page(content_humidity);
    content_add_page(content_voltage);
    content_add_page(content_interval);

    temperature_sensor_init();
    humidity_sensor_init();

    // Add task for display updating
    scheduler_add_task(display_loop_task, 0);
    scheduler_add_task(measure_sequence_task, 100);
    scheduler_add_task(node_start_task, 0);
}

void loop()
{
    handle_buttons();
    scheduler_loop();
}
