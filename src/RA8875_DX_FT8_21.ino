

#include <SPI.h>
#include <TimeLib.h>
#include <TinyGPS.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <si5351.h>

#include "RA8876_t3.h"
#include "Process_DSP.h"
#include "decode_ft8.h"
#include "WF_Table.h"
#include "arm_math.h"
#include "display.h"
#include "button.h"
#include "locator.h"
#include "traffic_manager.h"
#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"
#include "filters.h"
#include "constants.h"
#include "maidenhead.h"
#include "gen_ft8.h"
#include "options.h"
#include "ADIF.h"
#include "AudioSDRpreProcessor.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600
#define RA8876_CS 10
#define RA8876_RESET 9
#define BACKLITE 6 // My copy of the display is set for external backlight control
#define CTP_INT 27 // Use an interrupt capable pin such as pin 2 (any pin on a Teensy)

#define MAXTOUCHLIMIT 1

RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET);
Si5351 si5351;

TinyGPS gps;

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
AudioRecordQueue queue1;       // xy=1027,149

AudioAmplifier left_amp;
AudioAmplifier right_amp;

AudioConnection c1(i2s1, 0, preProcessor, 0);
AudioConnection c2(i2s1, 1, preProcessor, 1);

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
AudioConnection patchCord13(amp1, queue1);

AudioConnection c4(mixer1, 0, left_amp, 0);
AudioConnection c5(mixer2, 0, right_amp, 0);

AudioConnection c6(left_amp, 0, i2s2, 0);
AudioConnection c7(right_amp, 0, i2s2, 1);

AudioControlSGTL5000 sgtl5000_1; // xy=404,516

q15_t __attribute__((aligned(4))) dsp_buffer[3 * input_gulp_size];
q15_t __attribute__((aligned(4))) dsp_output[FFT_SIZE * 2];
q15_t __attribute__((aligned(4))) input_gulp[input_gulp_size * 5];

char Station_Call[11]; // six character call sign + /0
char Locator[11];      // four character locator  + /0

uint16_t currentFrequency;

uint32_t current_time, start_time, ft8_time;
uint32_t days_fraction, hours_fraction, minute_fraction;

uint8_t ft8_hours, ft8_minutes, ft8_seconds;
int ft8_flag, FT_8_counter, ft8_marker, decode_flag;
int WF_counter;
int num_decoded_msg;
int xmit_flag, ft8_xmit_counter, Transmit_Armned;
int DSP_Flag;
int master_decoded;

uint16_t cursor_freq;
uint16_t cursor_line;
int offset_freq;
int Tune_On;

float xmit_level = 0.8;
int logging_on;

int auto_location;
float flat, flon;
int32_t gps_latitude;
int32_t gps_longitude;
int8_t gps_hour;
int8_t gps_minute;
int8_t gps_second;
int8_t gps_hundred;
int8_t gps_offset = 2;

int SD_Year;
byte SD_Month, SD_Day, SD_Hour, SD_Minute, SD_Second, SD_Flag;

long et1 = 0, et2 = 0;

extern int CQ_Flag;
extern int Beacon_On;
extern int FT8_Touch_Flag;
extern int FT_8_TouchIndex;
extern int FT8_Message_Touch;
extern int FT_8_MessageIndex;
extern uint8_t RX_volume;
extern int RF_Gain;

extern uint16_t start_freq;

int QSO_xmit;
int Xmit_DSP_counter;
int slot_state = 0;
int target_slot;
int target_freq;

extern int16_t map_width;
extern int16_t map_heigth;
extern int16_t map_center_x;
extern int16_t map_center_y;
extern int Map_Index;

extern int offset_index;

time_t getTeensy3Time();
static void process_data();
static void parse_NEMA(void);
static void update_synchronization();
static void copy_to_fft_buffer(void *destination, const void *source);

void setup(void)
{
  Serial.begin(9600);

  if (CrashReport) 
  {
    Serial.print(CrashReport);
  }

  Serial1.begin(9600);

  init_RxSw_TxSw();

  setSyncProvider(getTeensy3Time);

  Serial.println("hello charley");

  delay(10);

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
  initalize_constants();

  set_startup_freq();

  AudioMemory(100);
  preProcessor.startAutoI2SerrorDetection();

  RX_volume = 10;
  RF_Gain = 20;

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(RX_volume);
  sgtl5000_1.lineOutLevel(31);
  sgtl5000_1.volume(0.4);

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

  left_amp.gain(0.2);
  right_amp.gain(0.2);

  queue1.begin();

  start_time = millis();

  open_stationData_file();

  set_Station_Coordinates(Locator);

  display_all_buttons();
  display_date(650, 30);
  display_station_data(820, 0);
  display_revision_level();

  display_value(620, 559, RF_Gain);

  Init_Log_File();
  draw_map(Map_Index);

  // update_synchronization();
}

void loop()
{
  if (decode_flag == 0)
    process_data();

  if (DSP_Flag == 1)
  {
    process_FT8_FFT();

    if (xmit_flag == 1)
    {
      if (Tune_On == 0)
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

  if (decode_flag == 1 && Tune_On == 0 && xmit_flag == 0)
  {
    // toggle the slot state
    slot_state = (slot_state == 0) ? 1 : 0;
    clear_decoded_messages();

    master_decoded = ft8_decode();
    if (master_decoded > 0)
      display_messages(master_decoded);

    if (Beacon_On == 1)
      service_Beacon_mode(master_decoded);
    else if (Beacon_On == 0)
      service_QSO_mode(master_decoded);

    decode_flag = 0;
  } // end of servicing FT_Decode

  if (Serial1.available())
    parse_NEMA();

  process_touch();

  if (Tune_On == 0 && FT8_Touch_Flag == 1 && Beacon_On == 0)
    process_selected_Station(master_decoded, FT_8_TouchIndex);

  update_synchronization();
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

static void process_data()
{
  if (queue1.available() >= num_que_blocks)
  {

    for (int i = 0; i < num_que_blocks; i++)
    {
      copy_to_fft_buffer(input_gulp + block_size * i, queue1.readBuffer());
      queue1.freeBuffer();
    }

    for (int i = 0; i < input_gulp_size; i++)
    {
      dsp_buffer[i] = dsp_buffer[i + input_gulp_size];
      dsp_buffer[i + input_gulp_size] = dsp_buffer[i + 2 * input_gulp_size];
      dsp_buffer[i + 2 * input_gulp_size] = input_gulp[i * 5]; // decimation by 5
    }

    DSP_Flag = 1;
  }
}

static void copy_to_fft_buffer(void *destination, const void *source)
{
  memcpy(destination, source, AUDIO_BLOCK_SAMPLES * sizeof(uint16_t));
}

void update_synchronization()
{
  current_time = millis();
  ft8_time = current_time - start_time;
  ft8_hours = (int8_t)(ft8_time / 3600000);
  hours_fraction = ft8_time % 3600000;
  ft8_minutes = (int8_t)(hours_fraction / 60000);
  ft8_seconds = (int8_t)((hours_fraction % 60000) / 1000);

  if (ft8_flag == 0 && ft8_time % 15000 <= 200)
  {
    ft8_flag = 1;
    FT_8_counter = 0;
    ft8_marker = 1;
    WF_counter = 0;

    show_decimal(680, 60, flat);
    show_decimal(880, 60, flon);

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

void parse_NEMA(void)
{
  while (Serial1.available())
  {
    if (gps.encode(Serial1.read()))
    { // process gps messages
      // when TinyGPS reports new data...
      unsigned long age;
      int Year;
      byte Month, Day, Hour, Minute, Second, Hundred;
      gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, &Hundred, &age);
      gps.f_get_position(&flat, &flon, &age);

      Second = Second + gps_offset;

      setTime(Hour, Minute, Second, Day, Month, Year);
      Teensy3Clock.set(now()); // set the RTC
      gps_hour = Hour;
      gps_minute = Minute;
      gps_second = Second;
      gps_hundred = Hundred;
      const char *locator = get_mh(flat, flon, 4);

      if (strindex(Locator, locator) < 0)
      {
        for (int i = 0; i < 11; i++)
          Locator[i] = locator[i];
        set_Station_Coordinates(Locator);
        display_station_data(820, 0);
      }
    }
  }
}
