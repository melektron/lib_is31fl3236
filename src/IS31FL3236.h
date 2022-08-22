/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
20.08.22, 21:12

A C++ HAL library to interact with the IS31FL3236(A) constant
current LED driver using the Arduino framework. 
*/

#pragma once

#define THREE_KHZ 0
#define TWEHTYTWO_KHZ 1

#include <Arduino.h>

class IS
{
protected:
    // address
    uint8_t devc_addr;
    // pin configuration
    int psdb;

    // constants:
    static constexpr int led_count = 36;

    // register definitions
    enum REG
    {
        REG_SD = 0x00,  // Shutdow register
        REG_PWM = 0x01, // first PWM register (range: 0x01 - 0x24)
        REG_UP = 0x25,  // (PWM) Update register (updates PWM and LED controll registers)
        REG_LC = 0x26,  // first LED controll register (range: 0x26 - 0x49)
        REG_GC = 0x4a,  // global controll register (not used)
        REG_OF = 0x4b,  // output frequency register
        REG_RS = 0x4f   // reset register
    };

    // cache of the LED controll registers
    uint8_t lc_cache[led_count] = {0};

protected:
    // shortcuts for controlling the SDB pin
    inline void SDB_L();
    inline void SDB_H();

    /**
     * @brief writes a byte to a register of the led controller
     *
     * @param _reg register address to write to
     * @param _val the byte to write to the register
     */
    void writeRegister(uint8_t _reg, uint8_t _val);

    /**
     * @brief writes a stream of bytes from a memory location to registers on the led controller
     * using the IS31FL3236A's address auto increment feature
     *
     * @param _reg_start register address to write the first byte to
     * @param _data pointer to the start of the memory block to be written
     * @param _len number of bytes to be written
     */
    void writeRegisters(uint8_t _reg_start, uint8_t *_data, size_t _len);

    /**
     * @brief writes the same value (byte) to multiple registers in a row on the led controller
     * using the IS31FL3236A's address auto increment feature
     *
     * @param _reg_start first register adress to write to
     * @param _val value to write to all the registers
     * @param _len amount of registers to be written
     */
    void writeRegisters(uint8_t _reg_start, uint8_t _val, size_t _len);

public:
    /**
     * @brief Construct a new IS31FL3236A controller instance
     * with the given pins and I2C address.
     *
     * @param _psdb (optional) SDB pin (set to -1 or leave at default to disable the use of the SDB pin)
     * @param _pscl (optional) SCL pin setting for microcontrollers that support pin remapping
     * @param _psda (optional) SDA pin setting for microcontrollers that support pin remapping
     */
    IS(uint8_t _addr, int _psdb = -1);

    /**
     * @brief initializes the led controller and all LED channels to the provided values.
     * the default state for all LEDs is OFF with the current setting set to
     * max current / 4 and a duty cycle of 0.
     *
     * @param _state state of the LEDs (true = ON, false = OFF)
     * @param _cur_div current devider for all LEDs (1, 2, 3 or 4)
     * @param _dc pwm duty cycle of the LEDs
     */
    void initialize(bool _state = false, uint8_t _cur_div = 1, uint8_t _dc = 0);

    /**
     * @brief makes any changes previously written to the PWM and
     * LED controll registers come into effect by writing
     * to the update register
     */
    void update();

    /**
     * @brief Set the global PWM output frequency
     *
     * @param _freq output frequency in kHz. The only allowed values are 3kHz and 22kHz.
     */
    void setFrequency(int _freq);

    /**
     * @brief disables all current outputs by writing to the shutdown
     * register. (aka soft shutdown)
     *
     */
    void shutdown();

    /**
     * @brief reenables all current outputs by writing to the shutdown
     * register. (aka disable soft shutdown)
     * 
     * NOTE: if the controller is in hard shutdown mode, hard shutdown mode 
     * will be disabled before soft enabeling the outputs
     *
     */
    void enable();

    /**
     * @brief engages a hard shutdown of the controller by pulling
     * the SDB pin low if the SDB pin is configured for the class instance
     * 
     */
    void hardShutdown();

    /**
     * @brief disengages a hard shutdown of the controller by pulling
     * the SDB pin high if the SDB pin is configured for the class instance.
     * 
     * NOTE: this will not soft enable outputs if normal (soft) shutdown
     * has been engaged before the hard shutdown.
     * 
     */
    void hardEnable();

    /**
     * @brief configures the current and state (on/off) of an individual channel.
     *
     * @param _ch channel number (1 - 36)
     * @param _state boolean (true/false) to set the state or -1 to keep the previous value
     * @param _cur_div current devider (1 - 4) to set the value or -1 to keep the previous value
     * @param _update flag defining whether the values should be updated immediately. By default,
     * the changes will only have an effect after calling update(). When set to true, update is called
     * internally. When configuring multiple LEDs, it is advised to leave this at false and call update()
     * once at the end.
     */
    void configure(uint8_t _ch, int8_t _state = -1, int8_t _cur_div = -1, bool _update = false);

    /**
     * @brief configures the current and state (on/off) of a RANGE of channels starting
     * with channel _ch_first and ending with channel _ch_last.
     *
     * @param _ch_first channel number of the first channel (1 - 36)
     * @param _ch_last channel number of the last channel (1 - 36)
     * @param _state boolean (true/false) to set the state or -1 to keep the previous value
     * @param _cur_div current devider (1 - 4) to set the value or -1 to keep the previous value
     * @param _update flag defining whether the values should be updated immediately. By default,
     * the changes will only have an effect after calling update(). When set to true, update is called
     * internally. When configuring multiple LEDs, it is advised to leave this at false and call update()
     * once at the end.
     */
    void multiconfigure(uint8_t _ch_first, uint8_t _ch_last, int8_t _state = -1, int8_t _cur_div = -1, bool _update = false);

    /**
     * @brief sets the pwm duty cycle of an individual channel
     *
     * @param _ch channel number (1 - 36)
     * @param _dc duty cycle to set
     * @param _update flag defining whether the values should be updated immediately. By default,
     * the changes will only have an effect after calling update(). When set to true, update is called
     * internally. When writing to multiple LEDs, it is advised to leave this at false and call update()
     * once at the end.
     */
    void write(uint8_t _ch, uint8_t _dc, bool _update = false);

    /**
     * @brief sets the pwm duty cycle of a RANGE of channels starting
     * with channel _ch_first and ending with channel _ch_last.
     *
     * @param _ch_first channel number of the first channel (1 - 36)
     * @param _ch_last channel number of the last channel (1 - 36)
     * @param _dc duty cycle to set
     * @param _update flag defining whether the values should be updated immediately. By default,
     * the changes will only have an effect after calling update(). When set to true, update is called
     * internally. When writing to multiple LEDs, it is advised to leave this at false and call update()
     * once at the end.
     */
    void multiwrite(uint8_t _ch_first, uint8_t _ch_last, uint8_t _dc, bool _update = false);

    /**
     * @brief sets the pwm duty cycle of a RANGE of channels to a number of different values
     * stored in an array of bytes. _dcs is a pointer to the start of the array containing the duty cycle
     * for the channels. The bytes will be written to the channels in numeric order starting with the 
     * lower of the provided channel numbers and working up to the higher channel number, no matter which
     * one is passed to _ch_first and _ch_last. To avoid confusion it is still recommended to pass the lower 
     * channel number to _ch_first and the higher channel number to _ch_last
     *
     * @param _ch_first channel number of the first channel (1 - 36)
     * @param _ch_last channel number of the last channel (1 - 36)
     * @param _dcs pointer to the duty cycle bytes
     * @param _update flag defining whether the values should be updated immediately. By default,
     * the changes will only have an effect after calling update(). When set to true, update is called
     * internally. When writing to multiple LEDs, it is advised to leave this at false and call update()
     * once at the end.
     */
    void buffermultiwrite(uint8_t _ch_first, uint8_t _ch_last, uint8_t *_dcs, bool _update = false);

    /**
     * @brief resets the led driver and all registers to their
     * default values by writing to the reset register
     */
    void reset();

    /**
     * @brief reset the I2C hardware by pulling the sdb pin
     * low for a short time. This is usefull in case of a
     * communication hang up.
     *
     * NOTE: this does NOT clear the configuration or register data
     */
    void hardReset();
};