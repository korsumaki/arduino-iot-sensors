#include <Arduino.h>
#include <scheduler.h>
#include <temperature_sensor.h>
#include <humidity_sensor.h>
#include <content.h> // Screen content handling


// Display: ESP8266 and ESP32 OLED driver for SSD1306 displays 
// 2x I2C Button (customizable I2C Address, default:0x31)
#include <LOLIN_I2C_BUTTON.h>
#include "SSD1306Wire.h"

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


const char * content1(void)
{
    static char screen_content_buffer[200] = { 0 };
    sprintf(screen_content_buffer, "Temperature:\n%.3f", temperature_get());
    return screen_content_buffer;
}
const char * content2(void)
{
    float pressure;
    float temp;
    float humidity;
    humidity_sensor_get(pressure, temp, humidity);

    static char screen_content_buffer[200] = { 0 };
    sprintf(screen_content_buffer, "%.2f%%\n%0.2fhPa\n%0.2f C", humidity, pressure, temp);
    return screen_content_buffer;
}
const char * content3(void)
{
    return "Voltage/n- V";
}

const char * content4(void)
{
    return "Mesh";
}

#define MEASUREMENT_SEQUENCE_INTERVAL_ms (2*1000) // development
//#define MEASUREMENT_SEQUENCE_INTERVAL_ms (10*60*1000) // production

static int measure_sequence_task(void)
{
    temperature_sensor_measure();
    humidity_sensor_measure();

    return MEASUREMENT_SEQUENCE_INTERVAL_ms;
}


void setup()
{
    Serial.begin(115200);
    scheduler_init(millis);

    init_display();
    content_init();
    content_add_page(content1);
    content_add_page(content2);
    content_add_page(content3);
    content_add_page(content4);

    temperature_sensor_init();
    humidity_sensor_init();

    // Add task for display updating
    scheduler_add_task(display_loop_task, 0);
    scheduler_add_task(measure_sequence_task, 100);
}

void loop()
{
    handle_buttons();
    scheduler_loop();
}
