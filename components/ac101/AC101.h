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

#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ac101 {

enum I2sSampleRate_t {
  SAMPLE_RATE_8000 = 0x0000,
  SAMPLE_RATE_11052 = 0x1000,
  SAMPLE_RATE_12000 = 0x2000,
  SAMPLE_RATE_16000 = 0x3000,
  SAMPLE_RATE_22050 = 0x4000,
  SAMPLE_RATE_24000 = 0x5000,
  SAMPLE_RATE_32000 = 0x6000,
  SAMPLE_RATE_44100 = 0x7000,
  SAMPLE_RATE_48000 = 0x8000,
  SAMPLE_RATE_96000 = 0x9000,
  SAMPLE_RATE_192000 = 0xa000,
};

enum I2sMode_t {
  MODE_MASTER = 0x00,
  MODE_SLAVE = 0x01,
};

enum I2sWordSize_t {
  WORD_SIZE_8_BITS = 0x00,
  WORD_SIZE_16_BITS = 0x01,
  WORD_SIZE_20_BITS = 0x02,
  WORD_SIZE_24_BITS = 0x03,
};

enum I2sFormat_t {
  DATA_FORMAT_I2S = 0x00,
  DATA_FORMAT_LEFT = 0x01,
  DATA_FORMAT_RIGHT = 0x02,
  DATA_FORMAT_DSP = 0x03,
};

enum I2sBitClockDiv_t {
  BCLK_DIV_1 = 0x0,
  BCLK_DIV_2 = 0x1,
  BCLK_DIV_4 = 0x2,
  BCLK_DIV_6 = 0x3,
  BCLK_DIV_8 = 0x4,
  BCLK_DIV_12 = 0x5,
  BCLK_DIV_16 = 0x6,
  BCLK_DIV_24 = 0x7,
  BCLK_DIV_32 = 0x8,
  BCLK_DIV_48 = 0x9,
  BCLK_DIV_64 = 0xa,
  BCLK_DIV_96 = 0xb,
  BCLK_DIV_128 = 0xc,
  BCLK_DIV_192 = 0xd,
};

enum I2sLrClockDiv_t {
  LRCK_DIV_16 = 0x0,
  LRCK_DIV_32 = 0x1,
  LRCK_DIV_64 = 0x2,
  LRCK_DIV_128 = 0x3,
  LRCK_DIV_256 = 0x4,
};

enum Mode_t { MODE_ADC, MODE_DAC, MODE_ADC_DAC, MODE_LINE };

class AC101 : public Component, public i2c::I2CDevice {
public:
  AC101() = default;

  void setup() override;

  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::LATE - 1; }

protected:
  // Get speaker volume.
  // @return Speaker volume, [63..0] for [0..-43.5] [dB], in increments of 2.
  uint8_t GetVolumeSpeaker();

  // Set speaker volume.
  // @param volume   Target volume, [63..0] for [0..-43.5] [dB], in increments
  // of 2.
  void SetVolumeSpeaker(uint8_t volume);

  // Get headphone volume.
  // @return Headphone volume, [63..0] for [0..-62] [dB]
  uint8_t GetVolumeHeadphone();

  // Set headphone volume
  // @param volume   Target volume, [63..0] for [0..-62] [dB]
  void SetVolumeHeadphone(uint8_t volume);

  // Configure I2S samplerate.
  // @param rate   Samplerate.
  void SetI2sSampleRate(I2sSampleRate_t rate);

  // Configure I2S mode (master/slave).
  // @param mode   Mode.
  void SetI2sMode(I2sMode_t mode);

  // Configure I2S word size (8/16/20/24 bits).
  // @param size   Word size.
  void SetI2sWordSize(I2sWordSize_t size);

  // Configure I2S format (I2S/Left/Right/Dsp).
  // @param format   I2S format.
  void SetI2sFormat(I2sFormat_t format);

  // Configure I2S clock.
  // @param bitClockDiv   I2S1CLK/BCLK1 ratio.
  // @param bitClockInv   I2S1 BCLK Polarity.
  // @param lrClockDiv    BCLK1/LRCK ratio.
  // @param lrClockInv    I2S1 LRCK Polarity.
  void SetI2sClock(I2sBitClockDiv_t bitClockDiv, bool bitClockInv,
                   I2sLrClockDiv_t lrClockDiv, bool lrClockInv);

  // Configure the mode (Adc/Dac/Adc+Dac/Line)
  // @param mode    Operating mode.
  void SetMode(Mode_t mode);

private:
  bool WriteReg(uint8_t reg, uint16_t val);
  bool ReadReg(uint8_t reg, uint16_t *val);
};

} // namespace ac101
} // namespace esphome