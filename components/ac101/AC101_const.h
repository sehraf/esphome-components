#pragma once

#include "AC101.h"

namespace esphome::ac101 {

#define AC101_ADDR 0x1A // Device address

static const uint8_t AC101_CHIP_AUDIO_RS = 0x00;
static const uint8_t AC101_PLL_CTRL1 = 0x01;
static const uint8_t AC101_PLL_CTRL2 = 0x02;
static const uint8_t AC101_SYSCLK_CTRL = 0x03;
static const uint8_t AC101_MOD_CLK_ENA = 0x04;
static const uint8_t AC101_MOD_RST_CTRL = 0x05;
static const uint8_t AC101_I2S_SR_CTRL = 0x06;
static const uint8_t AC101_I2S1LCK_CTRL = 0x10;
static const uint8_t AC101_I2S1_SDOUT_CTRL = 0x11;
static const uint8_t AC101_I2S1_SDIN_CTRL = 0x12;
static const uint8_t AC101_I2S1_MXR_SRC = 0x13;
static const uint8_t AC101_I2S1_VOL_CTRL1 = 0x14;
static const uint8_t AC101_I2S1_VOL_CTRL2 = 0x15;
static const uint8_t AC101_I2S1_VOL_CTRL3 = 0x16;
static const uint8_t AC101_I2S1_VOL_CTRL4 = 0x17;
static const uint8_t AC101_I2S1_MXR_GAIN = 0x18;
static const uint8_t AC101_ADC_DIG_CTRL = 0x40;
static const uint8_t AC101_ADC_VOL_CTRL = 0x41;
static const uint8_t AC101_HMIC_CTRL1 = 0x44;
static const uint8_t AC101_HMIC_CTRL2 = 0x45;
static const uint8_t AC101_HMIC_STATUS = 0x46;
static const uint8_t AC101_DAC_DIG_CTRL = 0x48;
static const uint8_t AC101_DAC_VOL_CTRL = 0x49;
static const uint8_t AC101_DAC_MXR_SRC = 0x4C;
static const uint8_t AC101_DAC_MXR_GAIN = 0x4D;
static const uint8_t AC101_ADC_APC_CTRL = 0x50;
static const uint8_t AC101_ADC_SRC = 0x51;
static const uint8_t AC101_ADC_SRCBST_CTRL = 0x52;
static const uint8_t AC101_OMIXER_DACA_CTRL = 0x53;
static const uint8_t AC101_OMIXER_SR = 0x54;
static const uint8_t AC101_OMIXER_BST1_CTRL = 0x55;
static const uint8_t AC101_HPOUT_CTRL = 0x56;
static const uint8_t AC101_SPKOUT_CTRL = 0x58;
static const uint8_t AC101_AC_DAC_DAPCTRL = 0xA0;
static const uint8_t AC101_AC_DAC_DAPHHPFC = 0xA1;
static const uint8_t AC101_AC_DAC_DAPLHPFC = 0xA2;
static const uint8_t AC101_AC_DAC_DAPLHAVC = 0xA3;
static const uint8_t AC101_AC_DAC_DAPLLAVC = 0xA4;
static const uint8_t AC101_AC_DAC_DAPRHAVC = 0xA5;
static const uint8_t AC101_AC_DAC_DAPRLAVC = 0xA6;
static const uint8_t AC101_AC_DAC_DAPHGDEC = 0xA7;
static const uint8_t AC101_AC_DAC_DAPLGDEC = 0xA8;
static const uint8_t AC101_AC_DAC_DAPHGATC = 0xA9;
static const uint8_t AC101_AC_DAC_DAPLGATC = 0xAA;
static const uint8_t AC101_AC_DAC_DAPHETHD = 0xAB;
static const uint8_t AC101_AC_DAC_DAPLETHD = 0xAC;
static const uint8_t AC101_AC_DAC_DAPHGKPA = 0xAD;
static const uint8_t AC101_AC_DAC_DAPLGKPA = 0xAE;
static const uint8_t AC101_AC_DAC_DAPHGOPA = 0xAF;
static const uint8_t AC101_AC_DAC_DAPLGOPA = 0xB0;
static const uint8_t AC101_AC_DAC_DAPOPT = 0xB1;
static const uint8_t AC101_DAC_DAP_ENA = 0xB5;

static const uint8_t AC101_REGS[] = {
    AC101_CHIP_AUDIO_RS,    AC101_PLL_CTRL1,        AC101_PLL_CTRL2,
    AC101_SYSCLK_CTRL,      AC101_MOD_CLK_ENA,      AC101_MOD_RST_CTRL,
    AC101_I2S_SR_CTRL,      AC101_I2S1LCK_CTRL,     AC101_I2S1_SDOUT_CTRL,
    AC101_I2S1_SDIN_CTRL,   AC101_I2S1_MXR_SRC,     AC101_I2S1_VOL_CTRL1,
    AC101_I2S1_VOL_CTRL2,   AC101_I2S1_VOL_CTRL3,   AC101_I2S1_VOL_CTRL4,
    AC101_I2S1_MXR_GAIN,    AC101_ADC_DIG_CTRL,     AC101_ADC_VOL_CTRL,
    AC101_HMIC_CTRL1,       AC101_HMIC_CTRL2,       AC101_HMIC_STATUS,
    AC101_DAC_DIG_CTRL,     AC101_DAC_VOL_CTRL,     AC101_DAC_MXR_SRC,
    AC101_DAC_MXR_GAIN,     AC101_ADC_APC_CTRL,     AC101_ADC_SRC,
    AC101_ADC_SRCBST_CTRL,  AC101_OMIXER_DACA_CTRL, AC101_OMIXER_SR,
    AC101_OMIXER_BST1_CTRL, AC101_HPOUT_CTRL,       AC101_SPKOUT_CTRL,
    AC101_AC_DAC_DAPCTRL,   AC101_AC_DAC_DAPHHPFC,  AC101_AC_DAC_DAPLHPFC,
    AC101_AC_DAC_DAPLHAVC,  AC101_AC_DAC_DAPLLAVC,  AC101_AC_DAC_DAPRHAVC,
    AC101_AC_DAC_DAPRLAVC,  AC101_AC_DAC_DAPHGDEC,  AC101_AC_DAC_DAPLGDEC,
    AC101_AC_DAC_DAPHGATC,  AC101_AC_DAC_DAPLGATC,  AC101_AC_DAC_DAPHETHD,
    AC101_AC_DAC_DAPLETHD,  AC101_AC_DAC_DAPHGKPA,  AC101_AC_DAC_DAPLGKPA,
    AC101_AC_DAC_DAPHGOPA,  AC101_AC_DAC_DAPLGOPA,  AC101_AC_DAC_DAPOPT,
    AC101_DAC_DAP_ENA};

} // namespace esphome::ac101
