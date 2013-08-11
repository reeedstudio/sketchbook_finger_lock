/*
  setKey.ino
  2013 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  2013-8-11
 
  write key to eeprom, address from 92 - 95, high first

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
#include <EEPROM.h>

#define DBG(X)      Serial.println(X)
#define FPON()      digitalWrite(A3, LOW)
#define FPOFF()     digitalWrite(A3, HIGH)
SoftwareSerial mySerial(A5, A4);                                // tx, rx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const unsigned long old_key_t  = 0x85112999;
const unsigned long new_key_t  = 0x85112991;

void write_eeprom()
{
    cout << "begin write to EEPROM, new_key = 0x";
    unsigned char new_key_s[4] ={(new_key_t>>24), (new_key_t>>16), (new_key_t>>8), new_key_t}; 
    for(int i=0; i<4; i++)
    {
        EEPROM.write(i+92, new_key_s[i]);
        Serial.print(new_key_s[i], HEX);
    }
    Serial.println();
    
    cout << "test read from eeprom: 0x";

    for(int i=0; i<4; i++)
    {
        Serial.print(EEPROM.read(92+i), HEX);
    }

    Serial.println();
}

int setKey()
{
    unsigned char packet[] = {0x12, (new_key_t>>24), (new_key_t>>16), (new_key_t>>8), new_key_t};        

    finger.writePacket(0xffffffff, FINGERPRINT_COMMANDPACKET, 7, packet);
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
    write_eeprom();
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

    if(setKey())
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

    Serial.begin(38400);
    DBG("hello world");
    cout << "begin to init wdt: 8s" << endl;

    pinMode(A3, OUTPUT);

    digitalWrite(A3, HIGH);

    FPOFF();
    finger.begin(19200);
    delay(10000);
    digitalWrite(A2, LOW);
    cout << "setup ok!" << endl;
    cout << "setkey_test" << endl;
    FPON();
    delay(1000);
    setKey_test();
}

void loop()                     // run over and over again
{
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
