/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
20.08.22, 21:12

A C++ HAL library to interact with the IS31FL3236(A) constant
current LED driver using the Arduino framework.
*/

#include "IS31FL3236.h"
#include <Wire.h>

#define THREE_KHZ 0
#define TWEHTYTWO_KHZ 1


inline void IS::SDB_L()
{
    if (psdb)
        digitalWrite(psdb, 0);
}

inline void IS::SDB_H()
{
    if (psdb)
        digitalWrite(psdb, 1);
}

void IS::writeRegister(uint8_t _reg, uint8_t _val)
{
    Wire.beginTransmission(devc_addr);
    Wire.write(_reg);
    Wire.write(_val);
    Wire.endTransmission();
}

void IS::writeRegisters(uint8_t _reg_start, uint8_t *_data, size_t _len)
{
    Wire.beginTransmission(devc_addr);
    Wire.write(_reg_start);
    for (size_t i = 0; i < _len; i++)
        Wire.write(*(_data + i));
    Wire.endTransmission();
}

void IS::writeRegisters(uint8_t _reg_start, uint8_t _val, size_t _len)
{
    Wire.beginTransmission(devc_addr);
    Wire.write(_reg_start);
    for (size_t i = 0; i < _len; i++)
        Wire.write(_val);
    Wire.endTransmission();
}

IS::IS(uint8_t _addr, int _psdb, int _pscl, int _psda)
    : devc_addr(_addr),
      psdb(_psdb),
      pscl(_pscl),
      psda(_psda)
{
}

void IS::initialize(bool _state, uint8_t _cur_div, uint8_t _dc)
{
    // current devider can only be 1, 2, 3 or 4
    if (_cur_div < 1 || _cur_div > 4)
        return;

    // configure SDB pin to output if one is provided
    if (psdb)
        pinMode(psdb, OUTPUT);

    // enable the chip using the SDB pin
    SDB_H();

    // initialize all LEDs to the provided state and current (default OFF and max current/4)
    uint8_t lc_val = ((_cur_div - 1) << 1) | (uint8_t)_state;
    writeRegisters(REG_LC, lc_val, led_count);
    memset(lc_cache, lc_val, led_count); // save the value to the internal cache
    // initialize the PWM registers
    writeRegisters(REG_PWM, _dc, led_count);

    // update so the changes have an effect
    update();
}

void IS::update()
{
    writeRegister(REG_UP, 0x00);
}

void IS::setFrequency(int _freq)
{
    if (_freq == 3)
        writeRegister(REG_OF, 0x00);
    else if (_freq == 22)
        writeRegister(REG_OF, 0x01);
}

void IS::shutdown()
{
    writeRegister(REG_SD, 0x00);
}

void IS::enable()
{
    SDB_H();
    writeRegister(REG_SD, 0x01);
}

void IS::hardShutdown()
{
    SDB_L();
}

void IS::hardEnable()
{
    SDB_H();
}

void IS::configure(uint8_t _ch, int8_t _state, int8_t _cur_div, bool _update)
{
    // check bounds of channel and provided configuration values
    if (
        _ch < 1 || _ch > 36 ||
        (_cur_div < 1 && _cur_div != -1) || _cur_div > 4 ||
        _state < -1 || _state > 1)
        return;

    // update the values in the cache if they are provided in the arguments
    if (_state != -1)
        lc_cache[_ch] = (lc_cache[_ch] & 0b11111110) | _state;
    if (_cur_div != -1)
        lc_cache[_ch] = (lc_cache[_ch] & 0b11111001) | ((_cur_div - 1) << 1);

    // write new cache value to register
    writeRegister(0x25 + _ch, lc_cache[_ch]);

    if (_update)
        update();
}

void IS::multiconfigure(uint8_t _ch_first, uint8_t _ch_last, int8_t _state, int8_t _cur_div, bool _update)
{
    // check bounds of channels and provided configuration values
    if (
        _ch_first < 1 || _ch_first > 36 ||
        _ch_last < 1 || _ch_last > 36 ||
        (_cur_div < 1 && _cur_div != -1) || _cur_div > 4 ||
        _state < -1 || _state > 1)
        return;

    // identify which channel number is the smaller and which one is the larger
    uint8_t chl = min(_ch_first, _ch_last);
    uint8_t chh = max(_ch_first, _ch_last);

    // update the values in the cache if they are provided in the arguments
    for (int ch = chl; ch < chh + 1; ch++)
    {
        if (_state != -1)
            lc_cache[ch] = (lc_cache[ch] & 0b11111110) | _state;
        if (_cur_div != -1)
            lc_cache[ch] = (lc_cache[ch] & 0b11111001) | ((_cur_div - 1) << 1);
    }

    // write the new cache values to the LEDs using address auto increment
    writeRegisters(0x25 + chl, (uint8_t *)(lc_cache + chl), chh - chl + 1);

    if (_update)
        update();
}

void IS::write(uint8_t _ch, uint8_t _dc, bool _update)
{
    if (_ch > 36 || _ch < 1)
        return;
    writeRegister(0x0 + _ch, _dc);

    if (_update)
        update();
}

void IS::multiwrite(uint8_t _ch_first, uint8_t _ch_last, uint8_t _dc, bool _update)
{
    // chech that the channel numbers are in bounds
    if (
        _ch_first < 1 || _ch_first > 36 ||
        _ch_last < 1 || _ch_last > 36)
        return;

    // identify which channel number is the smaller and which one is the larger
    uint8_t chl = min(_ch_first, _ch_last);
    uint8_t chh = max(_ch_first, _ch_last);

    // write the value to the registers using address auto increment
    writeRegisters(chl, _dc, chh - chl + 1);

    if (_update)
        update();
}

void IS::reset()
{
    writeRegister(0x4f, 0x00);
}

void IS::hardReset()
{
    SDB_L();
    delay(1);
    SDB_H();
}
