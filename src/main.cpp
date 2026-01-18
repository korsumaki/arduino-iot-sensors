#include <Arduino.h>
#include <scheduler.h>
#include <temperature_sensor.h>
#include <humidity_sensor.h>

int blink_task(void)
{
    static bool is_led_on = false;
    int time_for_next_call = 0;

    is_led_on = !is_led_on;
    if (is_led_on)
    {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("led on");
        time_for_next_call = 50;
    }
    else
    {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("led off");
        time_for_next_call = 500;
    }

    return time_for_next_call;
}

void setup()
{
    Serial.begin(115200);
    (void)scheduler_loop(millis());

    temperature_sensor_init();
    humidity_sensor_init();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    scheduler_add_task(blink_task, 0);

    temperature_sensor_measure();
    humidity_sensor_measure();
}

void loop()
{
    scheduler_loop(millis());
}
