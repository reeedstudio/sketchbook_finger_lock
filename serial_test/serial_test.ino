// serial test

#include <SoftwareSerial.h>


SoftwareSerial mySerial(A5, A4);                                // tx, rx

void setup()
{

    Serial.begin(38400);
    Serial.println("hello world");

    pinMode(A2, OUTPUT);
    pinMode(A3, OUTPUT);

    digitalWrite(A3, LOW);
    digitalWrite(A2, LOW);
    
    mySerial.begin(9600);

}

void loop()
{
    while(mySerial.available())
    {
        Serial.write(mySerial.read());
    }
    
    while(Serial.available())
    {
        mySerial.write(Serial.read());
    }
}