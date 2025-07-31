#include <Audio.h>
#include "RA8876_t3.h"
#include <TimeLib.h>
#include <si5351.h>

#include "Process_DSP.h"
#include "WF_Table.h"
#include "arm_math.h"
#include "decode_ft8.h"
#include "display.h"
#include "traffic_manager.h"
#include "button.h"
#include "main.h"

static uint8_t WF_index[ft8_buffer];
static float window[FFT_SIZE];

static q15_t __attribute__((aligned(4))) window_dsp_buffer[FFT_SIZE];
static q15_t FFT_Scale[FFT_SIZE * 2];
static q15_t FFT_Magnitude[FFT_SIZE];
static uint8_t FFT_Buffer[FFT_BASE_SIZE];
static float mag_db[FFT_BASE_SIZE + 1];

static arm_rfft_instance_q15 fft_inst;

static const size_t export_fft_power_size = ft8_msg_samples * ft8_buffer * 4;
uint8_t export_fft_power[export_fft_power_size];

static float ft_blackman_i(int i, int N)
{
  const float alpha = 0.16f; // or 2860/18608
  const float a0 = (1 - alpha) / 2;
  const float a1 = 1.0f / 2;
  const float a2 = alpha / 2;

  const float x1 = cosf(2 * (float)M_PI * i / (N - 1));
  const float x2 = 2 * x1 * x1 - 1; // Use double angle formula
  return a0 - a1 * x1 + a2 * x2;
}

void init_DSP(void)
{
  arm_rfft_init_q15(&fft_inst, FFT_SIZE, 0, 1);
  for (int i = 0; i < FFT_SIZE; ++i)
  {
    window[i] = ft_blackman_i(i, FFT_SIZE);
  }
}

// Compute FFT magnitudes (log power) for each timeslot in the signal
static void extract_power(size_t offset)
{
  int half_gulp = 0;
  for (int time_sub = 0; time_sub < 2; ++time_sub)
  {
    for (size_t i = 0; i < FFT_SIZE; i++)
    {
      window_dsp_buffer[i] = (q15_t)((float)dsp_buffer[i + half_gulp] * window[i]);
    }

    half_gulp += FFT_BASE_SIZE / 2;

    arm_rfft_q15(&fft_inst, window_dsp_buffer, dsp_output);
    arm_shift_q15(dsp_output, 5, FFT_Scale, FFT_SIZE * 2);
    arm_cmplx_mag_squared_q15(FFT_Scale, FFT_Magnitude, FFT_SIZE);

    for (int j = 0; j < FFT_BASE_SIZE; j++)
    {
      int32_t FFT_Mag_10 = 10 * (int32_t)FFT_Magnitude[j];
      mag_db[j] = 10.0f * log((float)FFT_Mag_10 + 0.1f);
    }

    // Loop over two possible frequency bin offsets (for averaging)
    for (int freq_sub = 0; freq_sub < 2; ++freq_sub)
    {
      for (int j = 0; j < ft8_buffer; ++j)
      {
        if (offset >= export_fft_power_size)
        {
          // Handle buffer overflow error
          break;
        }

        float db1 = mag_db[j * 2 + freq_sub];
        float db2 = mag_db[j * 2 + freq_sub + 1];
        int scaled = (int)((db1 + db2) / 2);

        export_fft_power[offset] = (scaled < 0) ? 0 : ((scaled > 255) ? 255 : scaled);
        ++offset;
      }
    }
  }
}

const int max_noise_count = 3;
const int max_noise_free_sets_count = 3;
static int noise_free_sets_count = 0;

static void update_offset_waterfall(int offset)
{
  for (int j = ft8_min_bin; j < ft8_buffer; j++)
    FFT_Buffer[j] = export_fft_power[j + offset];

  for (int x = ft8_min_bin; x < ft8_buffer; x++)
  {
    uint8_t bar = FFT_Buffer[x];
    if (bar > 63)
      bar = 63;

    WF_index[x] = bar / 4;
  }

  if (Auto_Sync)
  {
    // count the amount of noise seen in the FFT
    int noise_count = 0;

    for (int x = ft8_min_bin; x < ft8_buffer; x++)
    {
      if ((FFT_Buffer[x] > 0) && (++noise_count >= max_noise_count))
        break;
    }

    // if less than the maximum noise count in the FFT
    if (noise_count < max_noise_count)
    {
      // if sufficient noise free FFT data is seen it's time to synchronise
      if (++noise_free_sets_count >= max_noise_free_sets_count)
      {
        sync_FT8();
        Auto_Sync = 0;
        noise_free_sets_count = 0;
        sButtonData[6].state = 0;
        drawButton(6);
      }
    }
    else
    {
      noise_free_sets_count = 0;
    }
  }

  for (int k = ft8_min_bin; k < ft8_buffer; k++)
  {
    if (xmit_flag == 0)
      tft.drawPixel(2 * (k - ft8_min_bin), WF_counter, WFPalette[WF_index[k]]);
    else
    {
      if (2 * (k - ft8_min_bin) >= display_cursor_line && 2 * (k - ft8_min_bin) <= display_cursor_line + 16)
        tft.drawPixel(2 * (k - ft8_min_bin), WF_counter, WFPalette[WF_index[k]]);
      else
        tft.drawPixel(2 * (k - ft8_min_bin), WF_counter, BLACK);
    }

    tft.drawPixel(display_cursor_line, WF_counter, RED);
    tft.drawPixel(display_cursor_line + 16, WF_counter, RED);
  }

  WF_counter++;
}

void process_FT8_FFT(void)
{
  const int offset_step = ft8_buffer * 4;

  if (ft8_flag)
  {
    int master_offset = offset_step * FT_8_counter;
    extract_power(master_offset);

    update_offset_waterfall(master_offset);

    if (++FT_8_counter == ft8_msg_samples)
    {
      ft8_flag = 0;
      decode_flag = 1;
    }
  }
}
