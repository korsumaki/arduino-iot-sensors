#include <Arduino.h>
#include <scheduler.h>
#include <temperature_sensor.h>
#include <humidity_sensor.h>
#include <content.h> // Screen content handling


// Display: ESP8266 and ESP32 OLED driver for SSD1306 displays 
// 2x I2C Button (customizable I2C Address, default:0x31)
#include <LOLIN_I2C_BUTTON.h>
#include "SSD1306Wire.h"


#define MEASUREMENT_SEQUENCE_INTERVAL_ms (2*1000) // development
//#define MEASUREMENT_SEQUENCE_INTERVAL_ms (10*60*1000) // production


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
    sprintf(screen_content_buffer, "Temperature:\n%.2f C ", temperature_get());
    return screen_content_buffer;
}
const char * content_humidity(void)
{
    float pressure;
    float temp;
    float humidity;
    humidity_sensor_get(pressure, temp, humidity);

    static char screen_content_buffer[100] = { 0 };
    sprintf(screen_content_buffer, "%.2f%%\n%0.2f hPa\n%0.2f C ", humidity, pressure, temp);
    return screen_content_buffer;
}

int adc_value = 0; // TODO temporary
float adc_voltage = NAN; // TODO temporary

static float supply_voltage = NAN;

const char * content_voltage(void)
{
    static char screen_content_buffer[100] = { 0 };
    sprintf(screen_content_buffer, "Voltage:\n%0.2f V\n%d, %0.3f ", supply_voltage, adc_value, adc_voltage);
    //sprintf(screen_content_buffer, "Voltage:\n%0.2f V ", supply_voltage);
    return screen_content_buffer;
}

const char * content_node(void)
{
    // connected, sent packets, received packets
    return "Mesh";
}

const char * content_interval(void)
{
    static char screen_content_buffer[100] = { 0 };

    float interval_sec = MEASUREMENT_SEQUENCE_INTERVAL_ms/1000;
    if (interval_sec < 60)
    {
        sprintf(screen_content_buffer, "Interval:\n%.2f sec ", interval_sec);
    }
    else if (interval_sec < 60*60)
    {
        sprintf(screen_content_buffer, "Interval:\n%.2f min ", (interval_sec/60.0f));
    }
    else
    {
        sprintf(screen_content_buffer, "Interval:\n%.2f h ", (interval_sec/(60.0f*60.0f)));
    }

    return screen_content_buffer;
}

// MKO-01 rev0
#define VOLTAGE_MEAS_UPPER_RESISITOR (470.0f) // 470k
#define VOLTAGE_MEAS_LOWER_RESISITOR (100.0f) // 100k

float voltage_meas(void)
{
    // Constants
    const float V_REF = 3.3;     // Analog reference voltage (e.g., 5V or 3.3V)
    const float R_BITS = 10.0;   // ADC resolution (bits)
    const float ADC_STEPS = (1 << int(R_BITS)) - 1; // Number of steps (2^R_BITS - 1)

    /*int*/ adc_value = analogRead(A0);
    /*float*/ adc_voltage = (adc_value / ADC_STEPS) * V_REF; // Convert to voltage

    supply_voltage = adc_voltage * (VOLTAGE_MEAS_UPPER_RESISITOR+VOLTAGE_MEAS_LOWER_RESISITOR) / VOLTAGE_MEAS_LOWER_RESISITOR;

    Serial.printf("adc=%d, voltage=%f, supply_voltage=%f\n", adc_value, adc_voltage, supply_voltage);

    return supply_voltage;
}

static int measure_sequence_task(void)
{
    voltage_meas();
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
    content_add_page(content_temperature);
    content_add_page(content_humidity);
    content_add_page(content_voltage);
    content_add_page(content_node);
    content_add_page(content_interval);

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
