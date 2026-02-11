/*
        Edited for esphome

        AC101 - An AC101 Codec driver library for Arduino
        Copyright (C) 2019, Ivo Pullens, Emmission

        Inspired by:
        https://github.com/donny681/esp-adf/tree/master/components/audio_hal/driver/AC101

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "AC101.h"
#include "AC101_const.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <cinttypes>

namespace esphome {
namespace ac101 {

static const char *const TAG = "AC101";

// Mark the component as failed; use only in setup
#define AC101_ERROR_FAILED(func) \
  if (!(func)) { \
    this->mark_failed(); \
    return; \
  }

// Return false; use outside of setup
#define AC101_ERROR_CHECK(func) \
  if (!(func)) { \
    return false; \
  }

bool AC101::WriteReg(uint8_t reg, uint16_t value) {
  return this->write_bytes_16(reg, &value, 1);
}

bool AC101::ReadReg(uint8_t reg, uint16_t *value) {
  return this->read_bytes_16(reg, value, 1);
}

void AC101::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AC101...");

  // Reset all registers, readback default as sanity check
  AC101_ERROR_FAILED(this->WriteReg(AC101_CHIP_AUDIO_RS, 0x0123));
  delay(100);
  {
    uint16_t val;
    AC101_ERROR_FAILED(this->ReadReg(AC101_CHIP_AUDIO_RS, &val));
    if (val != 0x0101) {
      ESP_LOGE(TAG,
               "failed to reset AC101 (CHIP_AUDIO_RST=0x%04x, expected 0x0101)",
               val);
      this->mark_failed();
      return;
    }
  }

  AC101_ERROR_FAILED(this->WriteReg(AC101_SPKOUT_CTRL, 0xe880));

  // Enable the PLL from 256*44.1KHz MCLK source
  AC101_ERROR_FAILED(this->WriteReg(AC101_PLL_CTRL1, 0x014f));
  AC101_ERROR_FAILED(this->WriteReg(AC101_PLL_CTRL2, 0x8600));

  // Clocking system
  AC101_ERROR_FAILED(this->WriteReg(AC101_SYSCLK_CTRL, 0x8b08));
  AC101_ERROR_FAILED(this->WriteReg(AC101_MOD_CLK_ENA, 0x800c));
  AC101_ERROR_FAILED(this->WriteReg(AC101_MOD_RST_CTRL, 0x800c));

  // Set default at I2S, 44.1KHz, 16bit
  // this->SetI2sSampleRate(calculate_sample_rate_value(44100));
  this->SetI2sSampleRate(this->sample_frequency_);
  this->SetI2sClock(BCLK_DIV_8, false, LRCK_DIV_32, false);
  this->SetI2sMode(MODE_SLAVE);
  // this->SetI2sWordSize(AC101_RESOLUTION_16_BITS);
  this->SetI2sWordSize(this->resolution_);
  this->SetI2sFormat(DATA_FORMAT_I2S);

  // AIF config
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S1_SDOUT_CTRL, 0xc000));
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S1_SDIN_CTRL, 0xc000));
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S1_MXR_SRC, 0x2200));

  AC101_ERROR_FAILED(this->WriteReg(AC101_ADC_SRCBST_CTRL, 0xccc4));
  AC101_ERROR_FAILED(this->WriteReg(AC101_ADC_SRC, 0x2020));
  AC101_ERROR_FAILED(this->WriteReg(AC101_ADC_DIG_CTRL, 0x8000));
  AC101_ERROR_FAILED(this->WriteReg(AC101_ADC_APC_CTRL, 0xbbc3));

  // Path Configuration
  AC101_ERROR_FAILED(this->WriteReg(AC101_DAC_MXR_SRC, 0xcc00));
  AC101_ERROR_FAILED(this->WriteReg(AC101_DAC_DIG_CTRL, 0x8000));
  AC101_ERROR_FAILED(this->WriteReg(AC101_OMIXER_SR, 0x0081));
  AC101_ERROR_FAILED(this->WriteReg(AC101_OMIXER_DACA_CTRL, 0xf080));

  ESP_LOGCONFIG(TAG, "Configuring ADC_DAC, volume=%i%%", 100);
  this->SetMode(MODE_ADC_DAC);
  this->set_volume(1.0);

// #ifdef USE_SELECT
//   if (this->dac_output_select_ != nullptr) {
//     auto dac_power = this->get_dac_power();
//     if (dac_power.has_value()) {
//       auto dac_power_str = this->dac_output_select_->at(dac_power.value());
//       if (dac_power_str.has_value()) {
//         this->dac_output_select_->publish_state(dac_power_str.value());
//       } else {
//         ESP_LOGW(TAG, "Unknown DAC output power value: %d", dac_power.value());
//       }
//     }
//   }
//   if (this->adc_input_mic_select_ != nullptr) {
//     auto mic_input = this->get_mic_input();
//     if (mic_input.has_value()) {
//       auto mic_input_str = this->adc_input_mic_select_->at(mic_input.value());
//       if (mic_input_str.has_value()) {
//         this->adc_input_mic_select_->publish_state(mic_input_str.value());
//       } else {
//         ESP_LOGW(TAG, "Unknown ADC input mic value: %d", mic_input.value());
//       }
//     }
//   }
// #endif
}

uint8_t AC101::calculate_resolution_value(AC101Resolution resolution) {
  // AC101_RESOLUTION_8_BITS  = 0x00,
  // AC101_RESOLUTION_16_BITS = 0x01,
  // AC101_RESOLUTION_20_BITS = 0x02,
  // AC101_RESOLUTION_24_BITS = 0x03,
  switch (resolution) {
  case AC101_RESOLUTION_8_BITS:
    return 0x00;
  case AC101_RESOLUTION_16_BITS:
    return 0x01;
  case AC101_RESOLUTION_20_BITS:
    return 0x02;
  case AC101_RESOLUTION_24_BITS:
    return 0x03;
  default:
    return 0;
  }
}

uint16_t AC101::calculate_sample_rate_value(uint32_t sample_frequency) {
  // SAMPLE_RATE_8000 = 0x0000,
  // SAMPLE_RATE_11052 = 0x1000,
  // SAMPLE_RATE_12000 = 0x2000,
  // SAMPLE_RATE_16000 = 0x3000,
  // SAMPLE_RATE_22050 = 0x4000,
  // SAMPLE_RATE_24000 = 0x5000,
  // SAMPLE_RATE_32000 = 0x6000,
  // SAMPLE_RATE_44100 = 0x7000,
  // SAMPLE_RATE_48000 = 0x8000,
  // SAMPLE_RATE_96000 = 0x9000,
  // SAMPLE_RATE_192000 = 0xa000,
  switch (sample_frequency) {
  case 8000:
    return 0x0000;
  case 11052:
    return 0x1000;
  case 12000:
    return 0x2000;
  case 16000:
    return 0x3000;
  case 22050:
    return 0x4000;
  case 24000:
    return 0x5000;
  case 32000:
    return 0x6000;
  case 44100:
    return 0x7000;
  case 48000:
    return 0x8000;
  case 96000:
    return 0x9000;
  case 192000:
    return 0xa000;
  default:
    return 0x3000; // 16000
  }
}

void AC101::SetI2sSampleRate(uint32_t rate) {
  uint16_t reg_value = calculate_sample_rate_value(rate);
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S_SR_CTRL, reg_value));
}

void AC101::SetI2sMode(I2sMode_t mode) {
  uint16_t val;
  AC101_ERROR_FAILED(this->ReadReg(AC101_I2S1LCK_CTRL, &val));
  val &= ~0x8000;
  val |= uint16_t(mode) << 15;
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S1LCK_CTRL, val));
}

void AC101::SetI2sWordSize(AC101Resolution size) {
  uint16_t val;
  uint8_t resolution = calculate_resolution_value(size);

  AC101_ERROR_FAILED(this->ReadReg(AC101_I2S1LCK_CTRL, &val));
  val &= ~0x0030;
  val |= uint16_t(resolution) << 4;
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S1LCK_CTRL, val));
}

void AC101::SetI2sFormat(I2sFormat_t format) {
  uint16_t val;
  AC101_ERROR_FAILED(this->ReadReg(AC101_I2S1LCK_CTRL, &val));
  val &= ~0x000C;
  val |= uint16_t(format) << 2;
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S1LCK_CTRL, val));
}

void AC101::SetI2sClock(I2sBitClockDiv_t bitClockDiv, bool bitClockInv,
                        I2sLrClockDiv_t lrClockDiv, bool lrClockInv) {
  uint16_t val;
  AC101_ERROR_FAILED(this->ReadReg(AC101_I2S1LCK_CTRL, &val));
  val &= ~0x7FC0;
  val |= uint16_t(bitClockInv ? 1 : 0) << 14;
  val |= uint16_t(bitClockDiv) << 9;
  val |= uint16_t(lrClockInv ? 1 : 0) << 13;
  val |= uint16_t(lrClockDiv) << 6;
  AC101_ERROR_FAILED(this->WriteReg(AC101_I2S1LCK_CTRL, val));
}

void AC101::SetMode(Mode_t mode) {
  if (MODE_LINE == mode) {
    AC101_ERROR_FAILED(this->WriteReg(AC101_ADC_SRC, 0x0408));
    AC101_ERROR_FAILED(this->WriteReg(AC101_ADC_DIG_CTRL, 0x8000));
    AC101_ERROR_FAILED(this->WriteReg(AC101_ADC_APC_CTRL, 0x3bc0));
  }

  if ((MODE_ADC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    AC101_ERROR_FAILED(this->WriteReg(AC101_MOD_CLK_ENA, 0x800c));
    AC101_ERROR_FAILED(this->WriteReg(AC101_MOD_RST_CTRL, 0x800c));
  }

  if ((MODE_DAC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    // Enable Headphone output
    AC101_ERROR_FAILED(this->WriteReg(AC101_OMIXER_DACA_CTRL, 0xff80));
    delay(100);
    AC101_ERROR_FAILED(this->WriteReg(AC101_HPOUT_CTRL, 0xfbc0));

    // Enable Speaker output
    AC101_ERROR_FAILED(this->WriteReg(AC101_SPKOUT_CTRL, 0xeabd));
    delay(10);
  }
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
void AC101::dump_config() {
  ESP_LOGCONFIG(TAG, "AC101 Audio Codec:");
  LOG_I2C_DEVICE(this);

  // assume that both outputs have an equal level
  const int level = (int)(this->volume() * 100.0);
  ESP_LOGCONFIG(TAG, "  Volume: %i%%", level);
  ESP_LOGCONFIG(TAG, "  DAC Bits per Sample: %" PRIu8, this->resolution_);
  ESP_LOGCONFIG(TAG, "  Sample Rate: %" PRIu32, this->sample_frequency_);

// #ifdef USE_SELECT
//   LOG_SELECT("  ", "DacOutputSelect", this->dac_output_select_);
//   LOG_SELECT("  ", "ADCInputMicSelect", this->adc_input_mic_select_);
// #endif

#ifdef ESPHOME_LOG_HAS_VERBOSE
  ESP_LOGV(TAG, "  Register Values:");
  uint16_t value;
  for (size_t i = 0; i < ARRAY_SIZE(AC101_REGS); ++i) {
    uint8_t reg = AC101_REGS[i];
    AC101_READ_REG(reg, &value);
    ESP_LOGV(TAG, "    %02x = %04x", reg, value);
  }
#endif

  if (this->is_failed()) {
    ESP_LOGCONFIG(TAG, "  Failed to initialize");
    return;
  }
}
#undef ARRAY_SIZE

bool AC101::set_volume(float volume) {
  volume = clamp(volume, 0.0f, 1.0f);
  const uint8_t value = remap<uint8_t, float>(volume, 0.0f, 1.0f, 0, 63);

  uint16_t val;
  // treat speaker and headphones equally
  // headphones
  AC101_ERROR_CHECK(this->ReadReg(AC101_HPOUT_CTRL, &val));
  val &= ~63 << 4;
  val |= value << 4;
  AC101_ERROR_CHECK(this->WriteReg(AC101_HPOUT_CTRL, val));

  // speaker
  // Divide by 2, as it is scaled to same range as headphone volume
  AC101_ERROR_CHECK(this->ReadReg(AC101_SPKOUT_CTRL, &val));
  val &= ~31;
  val |= value >> 1;
  AC101_ERROR_CHECK(this->WriteReg(AC101_SPKOUT_CTRL, val));

  return true;
}

float AC101::volume() {
  uint16_t value;

  AC101_ERROR_CHECK(this->ReadReg(AC101_HPOUT_CTRL, &value));
  value = (value >> 4) & 63;

  return remap<float, uint8_t>(value, 0, 63, 0.0f, 1.0f);
}

bool AC101::set_mute_state_(bool mute_state) {
  this->is_muted_ = mute_state;
  uint16_t val, BITS;

  // headphones
  BITS = 0x3000; // bit 12 and 13
  AC101_ERROR_CHECK(this->ReadReg(AC101_HPOUT_CTRL, &val));
  if (mute_state) {
    // 0: Mute, 1: On
    val &= ~BITS;
  } else {
    val |= BITS;
  }
  AC101_ERROR_CHECK(this->WriteReg(AC101_HPOUT_CTRL, val));

  // speaker
  BITS = 0x0220; // bit 5 and 9
  AC101_ERROR_CHECK(this->ReadReg(AC101_SPKOUT_CTRL, &val));
  if (mute_state) {
    // 0: Disable; 1: Enable
    val &= ~BITS;
  } else {
    val |= BITS;
  }
  AC101_ERROR_CHECK(this->WriteReg(AC101_SPKOUT_CTRL, val));

  return true;
}

} // namespace ac101
} // namespace esphome