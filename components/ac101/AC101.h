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

#include "esphome/core/component.h"
#include "esphome/components/audio_dac/audio_dac.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace ac101 {

enum I2sMode_t {
  MODE_MASTER = 0x00,
  MODE_SLAVE = 0x01,
};

enum AC101Resolution : uint8_t {
  AC101_RESOLUTION_8_BITS = 8,
  AC101_RESOLUTION_16_BITS = 16, // little endian
  AC101_RESOLUTION_20_BITS = 32,
  AC101_RESOLUTION_24_BITS = 24,
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

class AC101 : public audio_dac::AudioDac,
              public Component,
              public i2c::I2CDevice {
public:
  AC101() = default;

  /////////////////////////
  // Component overrides //
  /////////////////////////
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  ////////////////////////
  // AudioDac overrides //
  ////////////////////////

  /// @brief Writes the volume out to the DAC
  /// @param volume floating point between 0.0 and 1.0
  /// @return True if successful and false otherwise
  bool set_volume(float volume) override;

  /// @brief Gets the current volume out from the DAC
  /// @return floating point between 0.0 and 1.0
  float volume() override;

  /// @brief Disables mute for audio out
  /// @return True if successful and false otherwise
  bool set_mute_off() override { return this->set_mute_state_(false); }

  /// @brief Enables mute for audio out
  /// @return True if successful and false otherwise
  bool set_mute_on() override { return this->set_mute_state_(true); }

  bool is_muted() override { return this->is_muted_; }

  //////////////////////////////////
  // AC101 configuration setters //
  //////////////////////////////////
  // void set_use_mclk(bool use_mclk) { this->use_mclk_ = use_mclk; }
  void set_bits_per_sample(AC101Resolution resolution) {
    this->resolution_ = resolution;
  }
  void set_sample_frequency(uint32_t sample_frequency) {
    this->sample_frequency_ = sample_frequency;
  }

  // void set_use_mic(bool use_mic) { this->use_mic_ = use_mic; }
  // void set_mic_gain(ES8311MicGain mic_gain) { this->mic_gain_ = mic_gain; }

protected:
  /// @brief Get speaker volume.
  /// @return Speaker volume, [63..0] for [0..-43.5] [dB], in increments of 2.
  uint8_t GetVolumeSpeaker();

  /// @brief Set speaker volume.
  /// @param volume Target volume, [63..0] for [0..-43.5] [dB], in increments
  /// of 2.
  void SetVolumeSpeaker(uint8_t volume);

  /// @brief Get headphone volume.
  /// @return Headphone volume, [63..0] for [0..-62] [dB]
  uint8_t GetVolumeHeadphone();

  /// @brief Set headphone volume
  /// @param volume Target volume, [63..0] for [0..-62] [dB]
  void SetVolumeHeadphone(uint8_t volume);

  /// @brief Configure I2S samplerate.
  /// @param rate Samplerate in Hz.
  void SetI2sSampleRate(uint32_t rate);

  /// @brief Configure I2S mode (master/slave).
  /// @param mode Mode.
  void SetI2sMode(I2sMode_t mode);

  /// @brief Configure I2S word size (8/16/20/24 bits).
  /// @param size Word size.
  void SetI2sWordSize(AC101Resolution size);

  /// @brief Configure I2S format (I2S/Left/Right/Dsp).
  /// @param format I2S format.
  void SetI2sFormat(I2sFormat_t format);

  /// @brief Configure I2S clock.
  /// @param bitClockDiv   I2S1CLK/BCLK1 ratio.
  /// @param bitClockInv   I2S1 BCLK Polarity.
  /// @param lrClockDiv    BCLK1/LRCK ratio.
  /// @param lrClockInv    I2S1 LRCK Polarity.
  void SetI2sClock(I2sBitClockDiv_t bitClockDiv, bool bitClockInv,
                   I2sLrClockDiv_t lrClockDiv, bool lrClockInv);

  // @brief Configure the mode (Adc/Dac/Adc+Dac/Line)
  // @param mode    Operating mode.
  void SetMode(Mode_t mode);

  /// @brief Mutes or unmute the DAC audio out
  /// @param mute_state True to mute, false to unmute
  /// @return
  bool set_mute_state_(bool mute_state);

  /// @brief Computes the register value for the configured sample rate
  /// @param sample_frequency frequency for both audio in and audio out
  /// @return register value
  static uint16_t calculate_sample_rate_value(uint32_t sample_frequency);

  /// @brief Computes the bits for the register for the configured resolution (bits per sample)
  /// @param resolution bits per sample enum for both audio in and audio out
  /// @return bit value
  static uint8_t calculate_resolution_value(AC101Resolution resolution);

private:
  bool WriteReg(uint8_t reg, uint16_t val);
  bool ReadReg(uint8_t reg, uint16_t *val);

  AC101Resolution resolution_;
  uint32_t sample_frequency_; // in Hz
};

} // namespace ac101
} // namespace esphome