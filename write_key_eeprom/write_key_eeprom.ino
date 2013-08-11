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

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Streaming.h>


#define __Debug         1                               // if debug mode


SoftwareSerial mySerial(A5, A4);                                // tx, rx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


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
