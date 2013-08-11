/*
  setKey.ino
  2013 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  2013-8-11
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <avr/wdt.h>

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <Servo.h>


#define __Debug         1                               // if debug mode
#define __WDT           0                               // if use wdt


#if __WDT
#define wdt_init(X)     wdt_enable(X)
#define feed()          wdt_reset()
#else
#define wdt_init(X)
#define feed()
#endif


#if __Debug
#define DBG(X)          Serial.println(X)
#else
#define DBG(X)
#endif

#define FPON()          digitalWrite(A3, LOW)
#define FPOFF()         digitalWrite(A3, HIGH)

SoftwareSerial mySerial(A5, A4);                                // tx, rx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo myservo;                                                  // create servo object to control a servo

const unsigned long old_key_t  = 0x99;
const unsigned long new_key_t  = 0x85112999;

const unsigned long old_addr_t = 0xffffffff;
const unsigned long new_addr_t = 0x00101020;

int setKey(unsigned long old_key, unsigned long new_key)
{
    unsigned char packet[] = {0x12, (new_key>>24), (new_key>>16), (new_key>>8), new_key};        

    finger.writePacket(old_addr_t, FINGERPRINT_COMMANDPACKET, 7, packet);
    int len = finger.getReply(packet);

    if(0xff == len)
    {
        DBG("get nothing!!");
    }

    if((1==len) && ((packet[0] == FINGERPRINT_ACKPACKET) && (packet[1] == FINGERPRINT_OK)))
    {
        return true;
    }

    return false;
}

void setKey_test()
{
    finger.setKey(old_key_t);
START:
    if(finger.verifyPassword())
    {
        cout << "verify password ok" << endl;

    }
    else
    {
        cout << "verify password no ok!" << endl;
        goto START; 
    }

    cout << "begin to set key" << endl;

    if(setKey(old_key_t, new_key_t))
    {
        cout << "set key ok" << endl;
    }
    else
    {
        cout << "set key no ok" << endl;
        while(1);
    }

    cout << "verify password again" << endl;


    if(finger.verifyPassword())
    {
        cout << "verify password ok" << endl;

    }
    else
    {
        cout << "verify password no ok!" << endl;
        while(1);
    }

    while(1);
}

int setAddr(unsigned char *old_addr, unsigned char *new_addr)
{

}

void setup()
{

#if __Debug
    Serial.begin(38400);
    DBG("hello world");
    cout << "begin to init wdt: 8s" << endl;
#endif

    pinMode(A2, OUTPUT);
    pinMode(A3, OUTPUT);

    digitalWrite(A3, HIGH);
    digitalWrite(A2, HIGH);

    FPOFF();
    finger.begin(19200);
    delay(10000);
    digitalWrite(A2, LOW);
#if __Debug
    cout << "setup ok!" << endl;
#endif
    cout << "setkey_test" << endl;
    FPON();
    delay(1000);
    setKey_test();
}

void loop()                     // run over and over again
{
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{

    feed();
    if (!finger.verifyPassword())
    {
        DBG("Did not find fingerprint sensor :(");
        return -1;
    }

    feed();
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)
    {
        return -1;
    }

    feed();
    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)
    {
        return -1;
    }

    feed();
    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)
    {
        return -1;
    }

#if __Debug
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    DBG(finger.confidence);
#endif

    feed();
    return finger.fingerID;

}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
