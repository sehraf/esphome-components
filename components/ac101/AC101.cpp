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

bool AC101::WriteReg(uint8_t reg, uint16_t value) {
  return this->write_bytes_16(reg, &value, 1);
}

uint16_t AC101::ReadReg(uint8_t reg) {
  uint16_t value;
  this->read_bytes_16(reg, &value, 1);
  return value;
}

void AC101::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AC101...");

  // Reset all registers, readback default as sanity check
  this->WriteReg(AC101_CHIP_AUDIO_RS, 0x0123);
  delay(100);
  // ok &= 0x0101 == ReadReg(CHIP_AUDIO_RS);

  this->WriteReg(AC101_SPKOUT_CTRL, 0xe880);

  // Enable the PLL from 256*44.1KHz MCLK source
  this->WriteReg(AC101_PLL_CTRL1, 0x014f);
  this->WriteReg(AC101_PLL_CTRL2, 0x8600);

  // Clocking system
  this->WriteReg(AC101_SYSCLK_CTRL, 0x8b08);
  this->WriteReg(AC101_MOD_CLK_ENA, 0x800c);
  this->WriteReg(AC101_MOD_RST_CTRL, 0x800c);

  // Set default at I2S, 44.1KHz, 16bit
  this->SetI2sSampleRate(SAMPLE_RATE_44100);
  this->SetI2sClock(BCLK_DIV_8, false, LRCK_DIV_32, false);
  this->SetI2sMode(MODE_SLAVE);
  this->SetI2sWordSize(WORD_SIZE_16_BITS);
  this->SetI2sFormat(DATA_FORMAT_I2S);

  // AIF config
  this->WriteReg(AC101_I2S1_SDOUT_CTRL, 0xc000);
  this->WriteReg(AC101_I2S1_SDIN_CTRL, 0xc000);
  this->WriteReg(AC101_I2S1_MXR_SRC, 0x2200);

  this->WriteReg(AC101_ADC_SRCBST_CTRL, 0xccc4);
  this->WriteReg(AC101_ADC_SRC, 0x2020);
  this->WriteReg(AC101_ADC_DIG_CTRL, 0x8000);
  this->WriteReg(AC101_ADC_APC_CTRL, 0xbbc3);

  // Path Configuration
  this->WriteReg(AC101_DAC_MXR_SRC, 0xcc00);
  this->WriteReg(AC101_DAC_DIG_CTRL, 0x8000);
  this->WriteReg(AC101_OMIXER_SR, 0x0081);
  this->WriteReg(AC101_OMIXER_DACA_CTRL, 0xf080);

  ESP_LOGCONFIG(TAG, "Configuring ADC_DAC, volume=90%%");
  this->SetMode(MODE_ADC_DAC);
  this->SetVolumeSpeaker(60);
  this->SetVolumeHeadphone(60);
}

uint8_t AC101::GetVolumeSpeaker() {
  // Times 2, to scale to same range as headphone volume
  return (ReadReg(AC101_SPKOUT_CTRL) & 31) * 2;
}

bool AC101::SetVolumeSpeaker(uint8_t volume) {
  // Divide by 2, as it is scaled to same range as headphone volume
  volume /= 2;
  if (volume > 31)
    volume = 31;

  uint16_t val = ReadReg(AC101_SPKOUT_CTRL);
  val &= ~31;
  val |= volume;
  return WriteReg(AC101_SPKOUT_CTRL, val);
}

uint8_t AC101::GetVolumeHeadphone() {
  return (ReadReg(AC101_HPOUT_CTRL) >> 4) & 63;
}

bool AC101::SetVolumeHeadphone(uint8_t volume) {
  if (volume > 63)
    volume = 63;

  uint16_t val = ReadReg(AC101_HPOUT_CTRL);
  val &= ~63 << 4;
  val |= volume << 4;
  return WriteReg(AC101_HPOUT_CTRL, val);
}

bool AC101::SetI2sSampleRate(I2sSampleRate_t rate) {
  return WriteReg(AC101_I2S_SR_CTRL, rate);
}

bool AC101::SetI2sMode(I2sMode_t mode) {
  uint16_t val = ReadReg(AC101_I2S1LCK_CTRL);
  val &= ~0x8000;
  val |= uint16_t(mode) << 15;
  return WriteReg(AC101_I2S1LCK_CTRL, val);
}

bool AC101::SetI2sWordSize(I2sWordSize_t size) {
  uint16_t val = ReadReg(AC101_I2S1LCK_CTRL);
  val &= ~0x0030;
  val |= uint16_t(size) << 4;
  return WriteReg(AC101_I2S1LCK_CTRL, val);
}

bool AC101::SetI2sFormat(I2sFormat_t format) {
  uint16_t val = ReadReg(AC101_I2S1LCK_CTRL);
  val &= ~0x000C;
  val |= uint16_t(format) << 2;
  return WriteReg(AC101_I2S1LCK_CTRL, val);
}

bool AC101::SetI2sClock(I2sBitClockDiv_t bitClockDiv, bool bitClockInv,
                        I2sLrClockDiv_t lrClockDiv, bool lrClockInv) {
  uint16_t val = ReadReg(AC101_I2S1LCK_CTRL);
  val &= ~0x7FC0;
  val |= uint16_t(bitClockInv ? 1 : 0) << 14;
  val |= uint16_t(bitClockDiv) << 9;
  val |= uint16_t(lrClockInv ? 1 : 0) << 13;
  val |= uint16_t(lrClockDiv) << 6;
  return WriteReg(AC101_I2S1LCK_CTRL, val);
}

bool AC101::SetMode(Mode_t mode) {
  bool ok = true;
  if (MODE_LINE == mode) {
    ok &= WriteReg(AC101_ADC_SRC, 0x0408);
    ok &= WriteReg(AC101_ADC_DIG_CTRL, 0x8000);
    ok &= WriteReg(AC101_ADC_APC_CTRL, 0x3bc0);
  }

  if ((MODE_ADC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    ok &= WriteReg(AC101_MOD_CLK_ENA, 0x800c);
    ok &= WriteReg(AC101_MOD_RST_CTRL, 0x800c);
  }

  if ((MODE_DAC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    // Enable Headphone output
    ok &= WriteReg(AC101_OMIXER_DACA_CTRL, 0xff80);
    delay(100);
    ok &= WriteReg(AC101_HPOUT_CTRL, 0xfbc0);
    ok &= SetVolumeHeadphone(30);

    // Enable Speaker output
    ok &= WriteReg(AC101_SPKOUT_CTRL, 0xeabd);
    delay(10);
    ok &= SetVolumeSpeaker(30);
  }
  return ok;
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
void AC101::dump_config() {
  ESP_LOGCONFIG(TAG, "AC101 Audio Codec:");

  // assume that both outputs have an equal level
  const int level = (int)(this->GetVolumeHeadphone() * 100.0 / 64.0);
  ESP_LOGCONFIG(TAG, "  Volume: %i%%", level);

#ifdef ESPHOME_LOG_HAS_VERBOSE
  ESP_LOGV(TAG, "  Register Values:");
  for (size_t i = 0; i < ARRAY_SIZE(AC101_REGS); ++i) {
    uint8_t reg = AC101_REGS[i];
    uint16_t value = this->ReadReg(reg);
    ESP_LOGV(TAG, "    %02x = %04x", reg, value);
  }
#endif
}

} // namespace ac101
} // namespace esphome