
/* Copyright (C) 2025 Paul Winwood G8KIG - All Rights Reserved. */

#include <memory.h>
#include <Wire.h>
#include <TimeLib.h>

// unnecessary dependencies
#include <Audio.h>
#include <RA8876_t3.h>
#include <si5351.h>

#include "main.h"
#include "PskInterface.h"

static const int MAX_SYNCTIME_RETRIES = 10;

static bool syncTime = true;
static int syncTimeCounter = 0;
static bool senderSent = false;

struct RTC_Time
{
  uint8_t seconds; // 00-59 in BCD
  uint8_t minutes; // 00-59 in BCD
  uint8_t hours;   // 00-23 in BCD
  uint8_t dayOfWeek;
  uint8_t day;
  uint8_t month;
  uint8_t year;
};

enum I2COperation
{
  OP_TIME_REQUEST = 0,
  OP_SENDER_RECORD,
  OP_SENDER_SOFTWARE_RECORD,
  OP_RECEIVER_RECORD,
  OP_SEND_REQUEST
};

static const uint8_t ESP32_I2C_ADDRESS = 0x2A;

void requestTimeSync(void)
{
  syncTime = true;
  // also cause the sender record to be sent again
  senderSent = false;
  syncTimeCounter = 0;
}

void getTime(void)
{
  if (syncTime && syncTimeCounter++ < MAX_SYNCTIME_RETRIES)
  {
    Wire1.beginTransmission(ESP32_I2C_ADDRESS);
    uint8_t retVal = Wire1.endTransmission();
    if (retVal == 0)
    {
      RTC_Time rtcTime;
      memset(&rtcTime, 0, sizeof(rtcTime));

      Wire1.beginTransmission(ESP32_I2C_ADDRESS);
      Wire1.write(OP_TIME_REQUEST);
      Wire1.endTransmission();
      Wire1.requestFrom(ESP32_I2C_ADDRESS, sizeof(RTC_Time));
      uint8_t *ptr = (uint8_t *)&rtcTime;
      size_t size = 0;
      for (;;)
      {
        if (Wire1.available())
        {
          ptr[size++] = Wire1.read();
          if (size >= sizeof(RTC_Time))
            break;
        }
      }

      if (rtcTime.year > 24 && rtcTime.year < 99 && size == sizeof(RTC_Time))
      {
        syncTime = false;

        Serial.printf("%u %2.2u:%2.2u:%2.2u %2.2u/%2.2u/%2.2u (%u)\n",
                      size,
                      rtcTime.hours, rtcTime.minutes, rtcTime.seconds,
                      rtcTime.day, rtcTime.month, rtcTime.year,
                      rtcTime.dayOfWeek);
        setTime(rtcTime.hours,
                rtcTime.minutes,
                rtcTime.seconds,
                rtcTime.day,
                rtcTime.month,
                rtcTime.year + 2000);
        Teensy3Clock.set(now()); // set the RTC
      }
    }
    else
    {
      Serial.printf("Failed to read RTC time: %u\n", retVal);
      syncTime = false;
    }
  }
}

bool addSenderRecord(const char *callsign, const char *gridSquare, const char *software)
{
  bool result = false;
  Wire1.beginTransmission(ESP32_I2C_ADDRESS);
  uint8_t retVal = Wire1.endTransmission();
  if (retVal == 0)
  {
    uint8_t buffer[32];
    size_t callsignLength = strlen(callsign);
    size_t gridSquareLength = strlen(gridSquare);

    size_t bufferSize = sizeof(uint8_t) + sizeof(uint8_t) + callsignLength + sizeof(uint8_t) + gridSquareLength;
    if (bufferSize < sizeof(buffer))
    {
      uint8_t *ptr = buffer;
      *ptr++ = (uint8_t)OP_SENDER_RECORD;
      // Add callsign as length-delimited
      *ptr++ = (uint8_t)callsignLength;
      memcpy(ptr, callsign, callsignLength);
      ptr += callsignLength;

      // Add gridSquare as length-delimited
      *ptr++ = (uint8_t)gridSquareLength;
      memcpy(ptr, gridSquare, gridSquareLength);
      ptr += gridSquareLength;

      Wire1.beginTransmission(ESP32_I2C_ADDRESS);
      Wire1.write(buffer, ptr - buffer);
      result = (Wire1.endTransmission() == 0);
    }

    if (result)
    {
      size_t softwareLength = strlen(software);
      bufferSize = sizeof(uint8_t) + sizeof(uint8_t) + softwareLength;
      if (bufferSize < sizeof(buffer))
      {
        uint8_t *ptr = buffer;
        *ptr++ = (uint8_t)OP_SENDER_SOFTWARE_RECORD;
        // Add software as length-delimited
        *ptr++ = (uint8_t)softwareLength;
        memcpy(ptr, software, softwareLength);
        ptr += softwareLength;

        Wire1.beginTransmission(ESP32_I2C_ADDRESS);
        Wire1.write(buffer, ptr - buffer);
        result = (Wire1.endTransmission() == 0);
      }
    }
  }
  return result;
}

bool addReceivedRecord(const char *callsign, uint32_t frequency, uint8_t snr)
{
  if (!senderSent)
  {
      addSenderRecord(Station_Call, Station_Locator, "DX FT8 Transceiver");
      senderSent = true;
  }

  bool result = false;
  Wire1.beginTransmission(ESP32_I2C_ADDRESS);
  uint8_t retVal = Wire1.endTransmission();
  if (retVal == 0)
  {
    uint8_t buffer[32];
    size_t callsignLength = strlen(callsign);
    size_t bufferSize = sizeof(uint8_t) + sizeof(uint8_t) + callsignLength + sizeof(uint32_t) + sizeof(uint8_t);
    if (bufferSize < sizeof(buffer))
    {
      uint8_t *ptr = buffer;
      *ptr++ = (uint8_t)OP_RECEIVER_RECORD;
      // Add callsign as length-delimited
      *ptr++ = (uint8_t)callsignLength;
      memcpy(ptr, callsign, callsignLength);
      ptr += callsignLength;

      // Add frequency
      memcpy(ptr, &frequency, sizeof(frequency));
      ptr += (uint8_t)sizeof(frequency);

      // Add SNR (1 byte)
      *ptr++ = snr;

      Wire1.beginTransmission(ESP32_I2C_ADDRESS);
      Wire1.write(buffer, ptr - buffer);
      result = (Wire1.endTransmission() == 0);
    }
  }
  return result;
}

bool sendRequest(void)
{
  Wire1.beginTransmission(ESP32_I2C_ADDRESS);
  Wire1.write(OP_SEND_REQUEST);
  return (Wire1.endTransmission() == 0);
}
