//
//    FILE: dht.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.14
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// HISTORY:
// 0.1.14 replace digital read with faster (~3x) code => more robust low MHz machines.
// 0.1.13 fix negative temperature
// 0.1.12 support DHT33 and DHT44 initial version
// 0.1.11 renamed DHTLIB_TIMEOUT
// 0.1.10 optimized faster WAKEUP + TIMEOUT
// 0.1.09 optimize size: timeout check + use of mask
// 0.1.08 added formula for timeout based upon clockspeed
// 0.1.07 added support for DHT21
// 0.1.06 minimize footprint (2012-12-27)
// 0.1.05 fixed negative temperature bug (thanks to Roseman)
// 0.1.04 improved readability of code using DHTLIB_OK in code
// 0.1.03 added error values for temp and humidity when read failed
// 0.1.02 added error codes
// 0.1.01 added support for Arduino 1.0, fixed typos (31/12/2011)
// 0.1.00 by Rob Tillaart (01/04/2011)
//
// inspired by DHT11 library
//
// Released to the public domain
//

#include "dhtlib_gpa788.h"

/////////////////////////////////////////////////////
//
// PUBLIC
//

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
DHTLIB_ErrorCode dhtlib_gpa788::read11()
{
    // READ VALUES
    DHTLIB_ErrorCode rv = _readSensor(DHTLIB_DHT11_WAKEUP);
    if (rv != DHTLIB_ErrorCode::DHTLIB_OK)
    {
        humidity    = static_cast<double>(DHTLIB_ErrorCode::DHTLIB_INVALID_VALUE); // invalid value, or is NaN prefered?
        temperature = static_cast<double>(DHTLIB_ErrorCode::DHTLIB_INVALID_VALUE); // invalid value
        return rv;
    }

    // CONVERT AND STORE
    humidity    = bits[0];  // bits[1] == 0;
    temperature = bits[2];  // bits[3] == 0;

    // TEST CHECKSUM
    // bits[1] && bits[3] both 0
    // Certains capteurs DHT11 transmettent le checksum dont la valeur inclut
    // la partie fractionnaire de la température et l'humidité relative
    // d'où la modification apportée ici. (TW, 2019)
    uint8_t sum = bits[0] + bits[2] + bits[1] + bits[3];
    if (bits[4] != sum) return DHTLIB_ErrorCode::DHTLIB_ERROR_CHECKSUM;

    return DHTLIB_ErrorCode::DHTLIB_OK;
}


bool dhtlib_gpa788::isConnected()
{
    return _readSensor(DHTLIB_DHT_WAKEUP) == DHTLIB_ErrorCode::DHTLIB_OK;
}

void dhtlib_gpa788::reset()
{
    setHumidity(static_cast<double>(DHTLIB_ErrorCode::DHTLIB_INVALID_VALUE));
    setTemperature(static_cast<double>(DHTLIB_ErrorCode::DHTLIB_INVALID_VALUE));
}

/////////////////////////////////////////////////////
//
// PRIVATE
//

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_TIMEOUT
DHTLIB_ErrorCode dhtlib_gpa788::_readSensor(uint8_t wakeupDelay)
{
    // INIT BUFFERVAR TO RECEIVE DATA
    uint8_t mask = 128;
    uint8_t idx = 0;

    // replace digitalRead() with Direct Port Reads.
    // reduces footprint ~100 bytes => portability issue?
    // direct port read is about 3x faster
	uint8_t bit = digitalPinToBitMask(connected_pin);
	uint8_t port = digitalPinToPort(connected_pin);
    volatile uint8_t *PIR = portInputRegister(port);

    // EMPTY BUFFER
    for (uint8_t i = 0; i < 5; i++) bits[i] = 0;

    // REQUEST SAMPLE
    pinMode(connected_pin, OUTPUT);
    digitalWrite(connected_pin, LOW); // T-be 
    delay(wakeupDelay);
    digitalWrite(connected_pin, HIGH);   // T-go
    delayMicroseconds(40);
    pinMode(connected_pin, INPUT);

    // GET ACKNOWLEDGE or TIMEOUT
    uint16_t loopCntLOW = DHTLIB_TIMEOUT;
    while ((*PIR & bit) == LOW )  // T-rel
    {
        if (--loopCntLOW == 0) return DHTLIB_ErrorCode::DHTLIB_ERROR_TIMEOUT;
    }

    uint16_t loopCntHIGH = DHTLIB_TIMEOUT;
    while ((*PIR & bit) != LOW )  // T-reh
    {
        if (--loopCntHIGH == 0) return DHTLIB_ErrorCode::DHTLIB_ERROR_TIMEOUT;
    }

    // READ THE OUTPUT - 40 BITS => 5 BYTES
    for (uint8_t i = 40; i != 0; i--)
    {
        loopCntLOW = DHTLIB_TIMEOUT;
        while ((*PIR & bit) == LOW )
        {
            if (--loopCntLOW == 0) return DHTLIB_ErrorCode::DHTLIB_ERROR_TIMEOUT;
        }

        uint32_t t = micros();

        loopCntHIGH = DHTLIB_TIMEOUT;
        while ((*PIR & bit) != LOW )
        {
            if (--loopCntHIGH == 0) return DHTLIB_ErrorCode::DHTLIB_ERROR_TIMEOUT;
        }

        if ((micros() - t) > 40)
        { 
            bits[idx] |= mask;
        }
        mask >>= 1;
        if (mask == 0)   // next byte?
        {
            mask = 128;
            idx++;
        }
    }
    pinMode(connected_pin, OUTPUT);
    digitalWrite(connected_pin, HIGH);

    return DHTLIB_ErrorCode::DHTLIB_OK;
}

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
DHTLIB_ErrorCode dhtlib_gpa788::_read()
{
    // READ VALUES
    DHTLIB_ErrorCode rv = _readSensor(DHTLIB_DHT11_WAKEUP);
    if (rv != DHTLIB_ErrorCode::DHTLIB_OK)
    {
        humidity    = static_cast<double>(DHTLIB_ErrorCode::DHTLIB_INVALID_VALUE);  // invalid value, or is NaN prefered?
        temperature = static_cast<double>(DHTLIB_ErrorCode::DHTLIB_INVALID_VALUE);  // invalid value
        return rv; // propagate error value
    }

    // CONVERT AND STORE
    humidity = word(bits[0], bits[1]) * 0.1;
    temperature = word(bits[2] & 0x7F, bits[3]) * 0.1;
    if (bits[2] & 0x80)  // negative temperature
    {
        temperature = -temperature;
    }

    // TEST CHECKSUM
    uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
    if (bits[4] != sum)
    {
        return DHTLIB_ErrorCode::DHTLIB_ERROR_CHECKSUM;
    }
    return DHTLIB_ErrorCode::DHTLIB_OK;
}
//
// END OF FILE
//
