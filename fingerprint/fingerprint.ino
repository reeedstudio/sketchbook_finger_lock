/*
  fingerprint.ino
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
#define __WDT           1                               // if use wdt


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

const long CHECKTIME  = 5000;                                   // ms
const int  angleServo = 60;                                     // Rotation angle

const int  NOISE_MIN  = 220;
const int  NOISE_MAX  = 500;
int bgNoise_xn = 150;

#if __WDT
void delay_wdt(long tms)
{
    //DBG("get in delay_wdt");
    long nt = tms/500;
    //cout << "nt = " << nt << endl;
    for(long i=0; i<nt; i++)
    {
        delay(500);
        feed();
    }
    delay(tms%500);
    feed();
}
#else
void delay_wdt(long tms)
{
    delay(tms);
}
#endif

int getAnalog()
{
    int sum=0;
    for(int i = 0; i<32; i++)
    {
        sum+=analogRead(A7);
    }
    return sum>>5;
}



int kick()
{
    int noise_t = getAnalog();

    if(noise_t>NOISE_MIN && noise_t<NOISE_MAX)
    {
        feed();
        return 1;
    }
    feed();
    return 0;
}

void open_close_door()
{
    myservo.attach(6);
    for(int i=20; i<angleServo; i++)
    {
        feed();
        myservo.write(i);
        delay_wdt(5);
    }

    delay_wdt(2000);

    for(int i=(angleServo-1); i>=20; i--)
    {
        feed();
        myservo.write(i);
        delay_wdt(5);
    }
    myservo.detach();
    feed();

}

void checkAndOpen()
{
    FPON();
    delay_wdt(1000);

    long t1 = millis();

#if __Debug
    cout << "checkAndOpen: t1 = " << t1 << endl;
#endif
    while(1)
    {
        if(getFingerprintIDez()>=0)
        {
            open_close_door();
            FPOFF();
            DBG("get right finger, open door now!!");
            feed();
            delay_wdt(5000);
            return;
        }

        long t2 = millis();
        long dt = t2 - t1;

        if(dt > CHECKTIME)
        {
#if __Debug
            cout << "checkAndOpen: t2 = " << t2 << endl;
            cout << "checkAndOpen: dt = " << dt << endl;
#endif
            DBG("timer out: didn't get right finger");
            break;
        }
        feed();
    }
    feed();

    FPOFF();
}

void setup()
{

#if __Debug
    Serial.begin(38400);
    DBG("hello world");
    cout << "begin to init wdt: 8s" << endl;
#endif

    finger.setKey(0x85112999);
    wdt_init(WDTO_8S);

    pinMode(A2, OUTPUT);
    pinMode(A3, OUTPUT);

    digitalWrite(A3, HIGH);
    digitalWrite(A2, HIGH);

    FPOFF();
    finger.begin(19200);
    feed();
    delay_wdt(5000);
    digitalWrite(A2, LOW);
    feed();
#if __Debug
    cout << "setup ok!" << endl;
#endif
}

void loop()                     // run over and over again
{
    static int kickTimes = 0;
    static long t1 = millis();

    if(kick())
    {
#if __Debug
        cout << "begin to check finger" << endl;
#endif
        checkAndOpen();
        delay_wdt(500);

        long t2 = millis();
#if __Debug&0
        cout << "dt = " << t2-t1 << endl;
#endif
        kickTimes++;

        if(kickTimes>= 3 && (t2-t1)<60000)               // if 3times within 1min
        {
            DBG("3time within 1min, delay for 1mins");
            delay_wdt(60000);
            t1 = millis();
            kickTimes = 0;
        }
        else if((t2-t1)>=60000)
        {
            DBG("excess 1min");
            kickTimes = 1;
            t1 = millis();
        }
        else
        {
            delay_wdt(5000);
        }
#if __Debug
        cout << "works again" << endl;
#endif
        feed();
    }
    feed();
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
