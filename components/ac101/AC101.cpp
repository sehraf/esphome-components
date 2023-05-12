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

#include "esphome.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

#define AC101_ADDR 0x1A // Device address

#define CHIP_AUDIO_RS 0x00
#define PLL_CTRL1 0x01
#define PLL_CTRL2 0x02
#define SYSCLK_CTRL 0x03
#define MOD_CLK_ENA 0x04
#define MOD_RST_CTRL 0x05
#define I2S_SR_CTRL 0x06
#define I2S1LCK_CTRL 0x10
#define I2S1_SDOUT_CTRL 0x11
#define I2S1_SDIN_CTRL 0x12
#define I2S1_MXR_SRC 0x13
#define I2S1_VOL_CTRL1 0x14
#define I2S1_VOL_CTRL2 0x15
#define I2S1_VOL_CTRL3 0x16
#define I2S1_VOL_CTRL4 0x17
#define I2S1_MXR_GAIN 0x18
#define ADC_DIG_CTRL 0x40
#define ADC_VOL_CTRL 0x41
#define HMIC_CTRL1 0x44
#define HMIC_CTRL2 0x45
#define HMIC_STATUS 0x46
#define DAC_DIG_CTRL 0x48
#define DAC_VOL_CTRL 0x49
#define DAC_MXR_SRC 0x4C
#define DAC_MXR_GAIN 0x4D
#define ADC_APC_CTRL 0x50
#define ADC_SRC 0x51
#define ADC_SRCBST_CTRL 0x52
#define OMIXER_DACA_CTRL 0x53
#define OMIXER_SR 0x54
#define OMIXER_BST1_CTRL 0x55
#define HPOUT_CTRL 0x56
#define SPKOUT_CTRL 0x58
#define AC_DAC_DAPCTRL 0xA0
#define AC_DAC_DAPHHPFC 0xA1
#define AC_DAC_DAPLHPFC 0xA2
#define AC_DAC_DAPLHAVC 0xA3
#define AC_DAC_DAPLLAVC 0xA4
#define AC_DAC_DAPRHAVC 0xA5
#define AC_DAC_DAPRLAVC 0xA6
#define AC_DAC_DAPHGDEC 0xA7
#define AC_DAC_DAPLGDEC 0xA8
#define AC_DAC_DAPHGATC 0xA9
#define AC_DAC_DAPLGATC 0xAA
#define AC_DAC_DAPHETHD 0xAB
#define AC_DAC_DAPLETHD 0xAC
#define AC_DAC_DAPHGKPA 0xAD
#define AC_DAC_DAPLGKPA 0xAE
#define AC_DAC_DAPHGOPA 0xAF
#define AC_DAC_DAPLGOPA 0xB0
#define AC_DAC_DAPOPT 0xB1
#define DAC_DAP_ENA 0xB5

namespace esphome {
namespace ac101 {

static const char *const TAG = "AC101";

bool AC101::WriteReg(uint8_t reg, uint16_t value) {
  std::array<uint8_t, 2> v = {uint8_t(value >> 8), uint8_t(value & 0xff)};
  return this->write_bytes(reg, v);
}

uint16_t AC101::ReadReg(uint8_t reg) {
  uint8_t a, b;
  this->read_byte(reg, &a, false);
  this->read_byte(reg, &b);
  return uint16_t(a) << 8 | uint16_t(b);
}

void AC101::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AC101...");

  // Reset all registers, readback default as sanity check
  this->WriteReg(CHIP_AUDIO_RS, 0x0123);
  delay(100);
  // ok &= 0x0101 == ReadReg(CHIP_AUDIO_RS);

  this->WriteReg(SPKOUT_CTRL, 0xe880);

  // Enable the PLL from 256*44.1KHz MCLK source
  this->WriteReg(PLL_CTRL1, 0x014f);
  this->WriteReg(PLL_CTRL2, 0x8600);

  // Clocking system
  this->WriteReg(SYSCLK_CTRL, 0x8b08);
  this->WriteReg(MOD_CLK_ENA, 0x800c);
  this->WriteReg(MOD_RST_CTRL, 0x800c);

  // Set default at I2S, 44.1KHz, 16bit
  this->SetI2sSampleRate(SAMPLE_RATE_44100);
  this->SetI2sClock(BCLK_DIV_8, false, LRCK_DIV_32, false);
  this->SetI2sMode(MODE_SLAVE);
  this->SetI2sWordSize(WORD_SIZE_16_BITS);
  this->SetI2sFormat(DATA_FORMAT_I2S);

  // AIF config
  this->WriteReg(I2S1_SDOUT_CTRL, 0xc000);
  this->WriteReg(I2S1_SDIN_CTRL, 0xc000);
  this->WriteReg(I2S1_MXR_SRC, 0x2200);

  this->WriteReg(ADC_SRCBST_CTRL, 0xccc4);
  this->WriteReg(ADC_SRC, 0x2020);
  this->WriteReg(ADC_DIG_CTRL, 0x8000);
  this->WriteReg(ADC_APC_CTRL, 0xbbc3);

  // Path Configuration
  this->WriteReg(DAC_MXR_SRC, 0xcc00);
  this->WriteReg(DAC_DIG_CTRL, 0x8000);
  this->WriteReg(OMIXER_SR, 0x0081);
  this->WriteReg(OMIXER_DACA_CTRL, 0xf080);

  ESP_LOGCONFIG(TAG, "Configuring ADC_DAC, volume=90%");
  this->SetMode(MODE_ADC_DAC);
  this->SetVolumeSpeaker(60);
  this->SetVolumeHeadphone(60);
}

uint8_t AC101::GetVolumeSpeaker() {
  // Times 2, to scale to same range as headphone volume
  return (ReadReg(SPKOUT_CTRL) & 31) * 2;
}

bool AC101::SetVolumeSpeaker(uint8_t volume) {
  // Divide by 2, as it is scaled to same range as headphone volume
  volume /= 2;
  if (volume > 31)
    volume = 31;

  uint16_t val = ReadReg(SPKOUT_CTRL);
  val &= ~31;
  val |= volume;
  return WriteReg(SPKOUT_CTRL, val);
}

uint8_t AC101::GetVolumeHeadphone() { return (ReadReg(HPOUT_CTRL) >> 4) & 63; }

bool AC101::SetVolumeHeadphone(uint8_t volume) {
  if (volume > 63)
    volume = 63;

  uint16_t val = ReadReg(HPOUT_CTRL);
  val &= ~63 << 4;
  val |= volume << 4;
  return WriteReg(HPOUT_CTRL, val);
}

bool AC101::SetI2sSampleRate(I2sSampleRate_t rate) {
  return WriteReg(I2S_SR_CTRL, rate);
}

bool AC101::SetI2sMode(I2sMode_t mode) {
  uint16_t val = ReadReg(I2S1LCK_CTRL);
  val &= ~0x8000;
  val |= uint16_t(mode) << 15;
  return WriteReg(I2S1LCK_CTRL, val);
}

bool AC101::SetI2sWordSize(I2sWordSize_t size) {
  uint16_t val = ReadReg(I2S1LCK_CTRL);
  val &= ~0x0030;
  val |= uint16_t(size) << 4;
  return WriteReg(I2S1LCK_CTRL, val);
}

bool AC101::SetI2sFormat(I2sFormat_t format) {
  uint16_t val = ReadReg(I2S1LCK_CTRL);
  val &= ~0x000C;
  val |= uint16_t(format) << 2;
  return WriteReg(I2S1LCK_CTRL, val);
}

bool AC101::SetI2sClock(I2sBitClockDiv_t bitClockDiv, bool bitClockInv,
                        I2sLrClockDiv_t lrClockDiv, bool lrClockInv) {
  uint16_t val = ReadReg(I2S1LCK_CTRL);
  val &= ~0x7FC0;
  val |= uint16_t(bitClockInv ? 1 : 0) << 14;
  val |= uint16_t(bitClockDiv) << 9;
  val |= uint16_t(lrClockInv ? 1 : 0) << 13;
  val |= uint16_t(lrClockDiv) << 6;
  return WriteReg(I2S1LCK_CTRL, val);
}

bool AC101::SetMode(Mode_t mode) {
  bool ok = true;
  if (MODE_LINE == mode) {
    ok &= WriteReg(ADC_SRC, 0x0408);
    ok &= WriteReg(ADC_DIG_CTRL, 0x8000);
    ok &= WriteReg(ADC_APC_CTRL, 0x3bc0);
  }

  if ((MODE_ADC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    ok &= WriteReg(MOD_CLK_ENA, 0x800c);
    ok &= WriteReg(MOD_RST_CTRL, 0x800c);
  }

  if ((MODE_DAC == mode) or (MODE_ADC_DAC == mode) or (MODE_LINE == mode)) {
    // Enable Headphone output
    ok &= WriteReg(OMIXER_DACA_CTRL, 0xff80);
    delay(100);
    ok &= WriteReg(HPOUT_CTRL, 0xfbc0);
    ok &= SetVolumeHeadphone(30);

    // Enable Speaker output
    ok &= WriteReg(SPKOUT_CTRL, 0xeabd);
    delay(10);
    ok &= SetVolumeSpeaker(30);
  }
  return ok;
}

} // namespace ac101
} // namespace esphome