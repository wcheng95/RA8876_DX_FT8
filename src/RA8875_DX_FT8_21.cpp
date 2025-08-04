

#include <SPI.h>
#include <TimeLib.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <si5351.h>
#include <RA8876_t3.h>

#include "arm_math.h"

#include "Arduino.h"
#include "Process_DSP.h"
#include "decode_ft8.h"
#include "WF_Table.h"
#include "display.h"
#include "button.h"
#include "traffic_manager.h"
#include "AudioStream.h"
#include "filters.h"
#include "constants.h"
#include "gen_ft8.h"
#include "options.h"
#include "ADIF.h"
#include "AudioSDRpreProcessor.h"
#include "main.h"
#include "Geodesy.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600
#define RA8876_CS 10
#define RA8876_RESET 9
#define BACKLITE 6 // My copy of the display is set for external backlight control
#define CTP_INT 27 // Use an interrupt capable pin such as pin 2 (any pin on a Teensy)

#define MAXTOUCHLIMIT 1

RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET);
Si5351 si5351;

AudioSDRpreProcessor preProcessor;
AudioInputI2S i2s1;            // xy=120,212
AudioEffectMultiply multiply2; // xy=285,414
AudioEffectMultiply multiply1; // xy=287,149
AudioSynthWaveformSine sine1;  // xy=289,244
AudioFilterFIR fir1;           // xy=471,153
AudioFilterFIR fir2;           // xy=472,413
AudioMixer4 mixer1;            // xy=666,173
AudioMixer4 mixer2;            // xy=675,406
AudioAmplifier amp1;           // xy=859,151
AudioOutputI2S i2s2;           // xy=868,258
AudioRecordQueue audioQueue;   // xy=1027,149

AudioAmplifier left_amp;
AudioAmplifier right_amp;

AudioAmplifier in_left_amp;
AudioAmplifier in_right_amp;

AudioConnection c11(i2s1, 0, in_left_amp, 0);
AudioConnection c12(i2s1, 1, in_right_amp, 0);

AudioConnection c13(in_left_amp, 0, preProcessor, 0);
AudioConnection c14(in_right_amp, 0, preProcessor, 1);

AudioConnection patchCord1(preProcessor, 0, multiply1, 0);
AudioConnection patchCord2(preProcessor, 1, multiply2, 1);
AudioConnection patchCord3(multiply2, fir2);
AudioConnection patchCord4(multiply1, fir1);
AudioConnection patchCord5(sine1, 0, multiply1, 1);
AudioConnection patchCord6(sine1, 0, multiply2, 0);
AudioConnection patchCord7(fir1, 0, mixer1, 0);
AudioConnection patchCord8(fir1, 0, mixer2, 0);
AudioConnection patchCord9(fir2, 0, mixer1, 1);
AudioConnection patchCord10(fir2, 0, mixer2, 1);

AudioConnection c3(mixer2, 0, amp1, 0);
AudioConnection patchCord13(amp1, audioQueue);

AudioConnection c4(mixer1, 0, left_amp, 0);
AudioConnection c5(mixer2, 0, right_amp, 0);

AudioConnection c6(left_amp, 0, i2s2, 0);
AudioConnection c7(right_amp, 0, i2s2, 1);

AudioControlSGTL5000 sgtl5000; // xy=404,516

q15_t __attribute__((aligned(4))) dsp_buffer[FFT_BASE_SIZE * 3];
q15_t __attribute__((aligned(4))) dsp_output[FFT_SIZE * 2];
q15_t __attribute__((aligned(4))) input_gulp[FFT_BASE_SIZE * 5];

char Station_Call[11];         // six character call sign + /0
char Station_Locator[7];       // up to six character locator  + /0
char Short_Station_Locator[5]; // four character locator  + /0

uint16_t currentFrequency;

uint32_t current_time, start_time;
int ft8_flag;
int FT_8_counter;
int ft8_marker;
int decode_flag;
int WF_counter;
int xmit_flag;
int ft8_xmit_counter;
int DSP_Flag;
int master_decoded;

uint16_t cursor_freq;
uint16_t cursor_line;
int Tune_On;

float xmit_level = 0.8;

int QSO_xmit;
int slot_state = 0;
int target_slot;

bool syncTime = true;

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
  OP_RECEIVER_RECORD,
  OP_SEND_REQUEST
};

static const uint8_t ESP32_I2C_ADDRESS = 0x2A;

static void process_data();
static void update_synchronization();
static void get_time();

void setup(void)
{
  Serial.begin(9600);

  if (CrashReport)
  {
    Serial.print(CrashReport);
  }

  init_RxSw_TxSw();

  setSyncProvider(getTeensy3Time);

  Serial.println("hello charley");

  delay(10);

  Wire.begin();
  Wire1.begin();
  pinMode(BACKLITE, OUTPUT);
  digitalWrite(BACKLITE, HIGH);

  tft.begin();
  tft.backlight(true);
  tft.useCapINT(CTP_INT); // we use the capacitive chip Interrupt out!
  tft.setTouchLimit(MAXTOUCHLIMIT);
  tft.enableCapISR(true); // capacitive touch screen interrupt it's armed
  tft.fillRect(0, 0, 1024, 600, BLACK);

  Init_BoardVersionInput();
  Check_Board_Version();
  Options_Initialize();

  start_Si5351();
  init_DSP();
  set_startup_freq();
  get_time();

  AudioMemory(100);
  preProcessor.startAutoI2SerrorDetection();

  RX_volume = 10;
  RF_Gain = 20;

  sgtl5000.enable();
  sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000.lineInLevel(RX_volume);
  sgtl5000.lineOutLevel(31);
  sgtl5000.volume(0.4);

  sine1.amplitude(1.0);
  sine1.frequency(10000);
  sine1.phase(0);

  fir1.begin(FIR_I, NUM_COEFFS);
  fir2.begin(FIR_Q, NUM_COEFFS);

  mixer1.gain(0, 0.4);
  mixer1.gain(1, -0.4);

  mixer2.gain(0, 0.4);
  mixer2.gain(1, 0.4);

  set_RF_Gain(RF_Gain);
  set_Attenuator_Gain(1.0);

  left_amp.gain(0.2);
  right_amp.gain(0.2);

  audioQueue.begin();

  start_time = millis();

  open_stationData_file();

  set_Station_Coordinates();

  LatLong ll = QRAtoLatLong(Station_Locator);
  if (ll.isValid)
  {
    show_decimal(680, 60, ll.latitude);
    show_decimal(880, 60, ll.longitude);
  }

  display_all_buttons();
  display_date(650, 30);
  display_station_data(820, 0);

  display_revision_level();

  display_value(620, 559, RF_Gain);

  Init_Log_File();
  draw_map(Map_Index);
  addSenderRecord(Station_Call, Station_Locator, "DX FT8 Xceiver");
}

void loop()
{
  const int offset_index = 8;
  if (!decode_flag)
    process_data();

  if (DSP_Flag)
  {
    process_FT8_FFT();

    if (xmit_flag)
    {
      if (!Tune_On)
      {
        if (ft8_xmit_counter >= offset_index && ft8_xmit_counter < 79 + offset_index)
        {
          set_FT8_Tone(tones[ft8_xmit_counter - offset_index]);
        }

        ft8_xmit_counter++;

        if (ft8_xmit_counter == 80 + offset_index)
        {
          xmit_flag = 0;
          terminate_transmit_armed();
        }
      }
    }

    display_time(880, 30);

    DSP_Flag = 0;
  }

  if (decode_flag && !Tune_On && !xmit_flag)
  {
    // toggle the slot state
    slot_state = (slot_state == 0) ? 1 : 0;
    clear_decoded_messages();

    master_decoded = ft8_decode();
    if (master_decoded > 0)
      display_messages(master_decoded);

    if (Beacon_On)
      service_Beacon_mode(master_decoded);
    else
      service_QSO_mode(master_decoded);

    decode_flag = 0;
  }

  process_touch();

  if (!Tune_On && FT8_Touch_Flag && !Beacon_On)
    process_selected_Station(master_decoded, FT_8_TouchIndex);

  update_synchronization();
  get_time();
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

static void process_data()
{
  if (audioQueue.available() >= num_que_blocks)
  {
    for (int i = 0; i < num_que_blocks; i++)
    {
      memcpy(input_gulp + block_size * i,
             audioQueue.readBuffer(),
             AUDIO_BLOCK_SAMPLES * sizeof(uint16_t));
      audioQueue.freeBuffer();
    }

    const size_t length = FFT_SIZE;
    memmove(dsp_buffer,
            dsp_buffer + FFT_BASE_SIZE,
            length * sizeof(q15_t));
    for (int i = 0; i < FFT_BASE_SIZE; i++)
    {
      dsp_buffer[length + i] = input_gulp[i * 5]; // decimation by 5
    }

    DSP_Flag = 1;
  }
}

void update_synchronization()
{
  current_time = millis();
  uint32_t ft8_time = current_time - start_time;
  if (ft8_flag == 0 && ft8_time % 15000 <= 200)
  {
    ft8_flag = 1;
    FT_8_counter = 0;
    ft8_marker = 1;
    WF_counter = 0;

    if (QSO_xmit == 1 && target_slot == slot_state)
    {
      setup_to_transmit_on_next_DSP_Flag();
      update_message_log_display(1);
      QSO_xmit = 0;
    }
  }
}

void sync_FT8(void)
{
  setSyncProvider(getTeensy3Time);
  Teensy3Clock.set(now()); // set the RTC
  start_time = millis();
  ft8_flag = 1;
  FT_8_counter = 0;
  ft8_marker = 1;
  WF_counter = 0;
}

static void get_time()
{
  if (syncTime)
  {
    Wire1.beginTransmission(ESP32_I2C_ADDRESS);
    uint8_t regVal = Wire1.endTransmission();
    if (regVal == 0)
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
      Serial.printf("Failed to read RTC time from ESP32 %u\n", regVal);
      syncTime = false;
    }
  }
}

bool addSenderRecord(const char *callsign, const char *gridSquare, const char *software)
{
  bool result = false;
  Wire1.beginTransmission(ESP32_I2C_ADDRESS);
  uint8_t regVal = Wire1.endTransmission();
  if (regVal == 0)
  {
    uint8_t buffer[32];
    size_t callsignLength = strlen(callsign);
    size_t gridSquareLength = strlen(gridSquare);
    size_t softwareLength = strlen(software);

    size_t bufferSize = sizeof(uint8_t) + sizeof(uint8_t) + callsignLength + sizeof(uint8_t) + gridSquareLength + sizeof(uint8_t) + softwareLength;
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

      // Add software as length-delimited
      *ptr++ = (uint8_t)softwareLength;
      memcpy(ptr, software, softwareLength);
      ptr += softwareLength;

      Wire1.beginTransmission(ESP32_I2C_ADDRESS);
      Wire1.write(buffer, ptr - buffer);
      result = (Wire1.endTransmission() == 0);
    }
  }
  return result;
}

bool addReceivedRecord(const char *callsign, uint32_t frequency, uint8_t snr)
{
  bool result = false;
  Wire1.beginTransmission(ESP32_I2C_ADDRESS);
  uint8_t regVal = Wire1.endTransmission();
  if (regVal == 0)
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

bool sendRequest()
{
  Wire1.beginTransmission(ESP32_I2C_ADDRESS);
  Wire1.write(OP_SEND_REQUEST);
  return (Wire1.endTransmission() == 0);
}
