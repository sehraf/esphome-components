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

#include "esphome.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ac101 {

static const char *const TAG = "AC101";

#define AC101_ERROR_CHECK(func)                                                \
  if (!(func)) {                                                               \
    this->mark_failed();                                                       \
    return;                                                                    \
  }
#define AC101_READ_REG(reg, value) AC101_ERROR_CHECK(this->ReadReg(reg, value));
#define AC101_WRITE_REG(reg, value)                                            \
  AC101_ERROR_CHECK(this->WriteReg(reg, value));

bool AC101::WriteReg(uint8_t reg, uint16_t value) {
  return this->write_bytes_16(reg, &value, 1);
}

bool AC101::ReadReg(uint8_t reg, uint16_t *value) {
  return this->read_bytes_16(reg, value, 1);
}

void AC101::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AC101...");

  // Reset all registers, readback default as sanity check
  AC101_WRITE_REG(AC101_CHIP_AUDIO_RS, 0x0123);
  delay(100);
  {
    uint16_t val;
    AC101_READ_REG(AC101_CHIP_AUDIO_RS, &val);
    if (val != 0x0101) {
      ESP_LOGE(TAG,
               "failed to reset AC101 (CHIP_AUDIO_RST=0x%04x, expected 0x0101)",
               val);
      this->mark_failed();
      return;
    }
  }

  AC101_WRITE_REG(AC101_SPKOUT_CTRL, 0xe880);

  // Enable the PLL from 256*44.1KHz MCLK source
  AC101_WRITE_REG(AC101_PLL_CTRL1, 0x014f);
  AC101_WRITE_REG(AC101_PLL_CTRL2, 0x8600);

  // Clocking system
  AC101_WRITE_REG(AC101_SYSCLK_CTRL, 0x8b08);
  AC101_WRITE_REG(AC101_MOD_CLK_ENA, 0x800c);
  AC101_WRITE_REG(AC101_MOD_RST_CTRL, 0x800c);

  // Set default at I2S, 44.1KHz, 16bit
  this->SetI2sSampleRate(SAMPLE_RATE_44100);
  this->SetI2sClock(BCLK_DIV_8, false, LRCK_DIV_32, false);
  this->SetI2sMode(MODE_SLAVE);
  this->SetI2sWordSize(WORD_SIZE_16_BITS);
  this->SetI2sFormat(DATA_FORMAT_I2S);

  // AIF config
  AC101_WRITE_REG(AC101_I2S1_SDOUT_CTRL, 0xc000);
  AC101_WRITE_REG(AC101_I2S1_SDIN_CTRL, 0xc000);
  AC101_WRITE_REG(AC101_I2S1_MXR_SRC, 0x2200);

  AC101_WRITE_REG(AC101_ADC_SRCBST_CTRL, 0xccc4);
  AC101_WRITE_REG(AC101_ADC_SRC, 0x2020);
  AC101_WRITE_REG(AC101_ADC_DIG_CTRL, 0x8000);
  AC101_WRITE_REG(AC101_ADC_APC_CTRL, 0xbbc3);

  // Path Configuration
  AC101_WRITE_REG(AC101_DAC_MXR_SRC, 0xcc00);
  AC101_WRITE_REG(AC101_DAC_DIG_CTRL, 0x8000);
  AC101_WRITE_REG(AC101_OMIXER_SR, 0x0081);
  AC101_WRITE_REG(AC101_OMIXER_DACA_CTRL, 0xf080);

  ESP_LOGCONFIG(TAG, "Configuring ADC_DAC, volume=%i%%", 100);
  this->SetMode(MODE_ADC_DAC);
  this->SetVolumeSpeaker(63);
  this->SetVolumeHeadphone(63);
}

uint8_t AC101::GetVolumeSpeaker() {
  uint16_t val;
  // can't use macro here
  if (!(this->ReadReg(AC101_SPKOUT_CTRL, &val))) {
    this->mark_failed();
    return 0;
  }
  // Times 2, to scale to same range as headphone volume
  return (val & 31) * 2;
}

void AC101::SetVolumeSpeaker(uint8_t volume) {
  // Divide by 2, as it is scaled to same range as headphone volume
  volume /= 2;
  if (volume > 31)
    volume = 31;

  uint16_t val;
  AC101_READ_REG(AC101_SPKOUT_CTRL, &val);
  val &= ~31;
  val |= volume;
  AC101_WRITE_REG(AC101_SPKOUT_CTRL, val);
}

uint8_t AC101::GetVolumeHeadphone() {
  uint16_t val;
  // can't use macro here due to return value
  if (!(this->ReadReg(AC101_HPOUT_CTRL, &val))) {
    this->mark_failed();
    return 0;
  }
  return (val >> 4) & 63;
}

void AC101::SetVolumeHeadphone(uint8_t volume) {
  if (volume > 63)
    volume = 63;

  uint16_t val;
  AC101_READ_REG(AC101_HPOUT_CTRL, &val);
  val &= ~63 << 4;
  val |= volume << 4;
  AC101_WRITE_REG(AC101_HPOUT_CTRL, val);
}

void AC101::SetI2sSampleRate(I2sSampleRate_t rate) {
  AC101_WRITE_REG(AC101_I2S_SR_CTRL, rate);
}

void AC101::SetI2sMode(I2sMode_t mode) {
  uint16_t val;
  AC101_READ_REG(AC101_I2S1LCK_CTRL, &val);
  val &= ~0x8000;
  val |= uint16_t(mode) << 15;
  AC101_WRITE_REG(AC101_I2S1LCK_CTRL, val);
}

void AC101::SetI2sWordSize(I2sWordSize_t size) {
  uint16_t val;
  AC101_READ_REG(AC101_I2S1LCK_CTRL, &val);
  val &= ~0x0030;
  val |= uint16_t(size) << 4;
  AC101_WRITE_REG(AC101_I2S1LCK_CTRL, val);
}

void AC101::SetI2sFormat(I2sFormat_t format) {
  uint16_t val;
  AC101_READ_REG(AC101_I2S1LCK_CTRL, &val);
  val &= ~0x000C;
  val |= uint16_t(format) << 2;
  AC101_WRITE_REG(AC101_I2S1LCK_CTRL, val);
}

void AC101::SetI2sClock(I2sBitClockDiv_t bitClockDiv, bool bitClockInv,
                        I2sLrClockDiv_t lrClockDiv, bool lrClockInv) {
  uint16_t val;
  AC101_READ_REG(AC101_I2S1LCK_CTRL, &val);
  val &= ~0x7FC0;
  val |= uint16_t(bitClockInv ? 1 : 0) << 14;
  val |= uint16_t(bitClockDiv) << 9;
  val |= uint16_t(lrClockInv ? 1 : 0) << 13;
  val |= uint16_t(lrClockDiv) << 6;
  AC101_WRITE_REG(AC101_I2S1LCK_CTRL, val);
}

void AC101::SetMode(Mode_t mode) {
  if (MODE_LINE == mode) {
    AC101_WRITE_REG(AC101_ADC_SRC, 0x0408);
    AC101_WRITE_REG(AC101_ADC_DIG_CTRL, 0x8000);
    AC101_WRITE_REG(AC101_ADC_APC_CTRL, 0x3bc0);
  }

  if ((MODE_ADC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    AC101_WRITE_REG(AC101_MOD_CLK_ENA, 0x800c);
    AC101_WRITE_REG(AC101_MOD_RST_CTRL, 0x800c);
  }

  if ((MODE_DAC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    // Enable Headphone output
    AC101_WRITE_REG(AC101_OMIXER_DACA_CTRL, 0xff80);
    delay(100);
    AC101_WRITE_REG(AC101_HPOUT_CTRL, 0xfbc0);
    SetVolumeHeadphone(30);

    // Enable Speaker output
    AC101_WRITE_REG(AC101_SPKOUT_CTRL, 0xeabd);
    delay(10);
    SetVolumeSpeaker(30);
  }
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
void AC101::dump_config() {
  ESP_LOGCONFIG(TAG, "AC101 Audio Codec:");

  // assume that both outputs have an equal level
  const int level = (int)(this->GetVolumeHeadphone() * 100.0 / 63.0);
  ESP_LOGCONFIG(TAG, "  Volume: %i%%", level);

#ifdef ESPHOME_LOG_HAS_VERBOSE
  ESP_LOGV(TAG, "  Register Values:");
  uint16_t value;
  for (size_t i = 0; i < ARRAY_SIZE(AC101_REGS); ++i) {
    uint8_t reg = AC101_REGS[i];
    AC101_READ_REG(reg, &value);
    ESP_LOGV(TAG, "    %02x = %04x", reg, value);
  }
#endif
}

} // namespace ac101
} // namespace esphome