#include <Arduino.h>
#include <scheduler.h>
#include <temperature_sensor.h>
#include <humidity_sensor.h>
#include <screen.h> // Screen content handling


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

uint8_t screen_page_index = 0;
uint8_t screen_scroll_index = 0;

void button_a_pressed(void)
{
    screen_page_index++;
    screen_scroll_index = 0;
}

void button_b_pressed(void)
{
    screen_scroll_index++;
}

void display_loop(void)
{
    if (is_display_available)
    {
        display.cls();
        if (screen_page_index >= screen_get_count())
        {
            screen_page_index = 0;
        }
        display.printf("%s", screen_get_screen(screen_page_index));
        //display.println("IoT mesh 2");
        //display.printf("%d %d\n", screen_page_index, screen_scroll_index);
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


const char * screen1(void)
{
    return "Temperature\n12.4 C";
}
const char * screen2(void)
{
    return "Humidity\n65%%";
}
const char * screen3(void)
{
    return "Mesh";
}


void setup()
{
    Serial.begin(115200);
    (void)scheduler_loop(millis());

    init_display();
    screen_init();
    screen_add_screen(screen1);
    screen_add_screen(screen2);
    screen_add_screen(screen3);

    //temperature_sensor_init();
    //humidity_sensor_init();

    //scheduler_add_task(blink_task, 0);

    //temperature_sensor_measure();
    //humidity_sensor_measure();
}

void loop()
{
    handle_buttons();
    display_loop();
    delay(200);
    scheduler_loop(millis());
}
