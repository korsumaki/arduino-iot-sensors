#include <Arduino.h>

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("led on");
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("led off");
    delay(1000);
}
