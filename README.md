# Disclaimer

This repository is under construction. The most important functionality is already working, but some things might change and more features will be added. Also some more documentation and examples will be added in the next couple of days.

<br><br>

# Lib IS31FL3236

Lib IS31FL3236 is a simple C++ library to more easily controll the ISSI IS31FL3236 and IS31FL3236A constant current LED driver ICs.

The library works with IDEs based on the Arduino framework but is mostly targeted to the PlatformIO IDE. The Arduino IDE should also work.

# IS31FL3236(a)

The IS31FL3236(A) IC is a simple I2C device that can drive up to 36 individual channels with a constant current of up to 38 mA per channel (given there is sufficient heat dissipation).

The the output current can be configured globally using an external resistor connected between the R_EXT pin and GND. A typical value is 3.9k which results in an output current of just under 20 mA.

In case of the A variant of the IC, the PWM frequency can also be configured to either 3 kHz or 22 kHz using software.

The IC also offers a softare shutdown mode to quickly turn off all channel's current sources without changing their configuration. When software shutdown mode is deactivated, operation goes back to normal with all channels retaining their state, current and duty cycle. During software shutdown mode, the chip can be operated normally except all the current sources are switched off.

There is also a hardware shutdown mode that can be activated by pulling the SDB pin low. During hardware shutdown mode, all current sources as well as the I2C interface are disabled. During this state, the chip can not be operated but all the register information and configuration still remains and the operation will continue like normal after hardware shutdown mode is disabled.

## Channels

The following settings can be configured for every channel **individually** and can be changed at any time:
 * `state` (**bool**: true/false): enables or disables the channel. When disabled, the channel can still be configured but the current source of the channel will be turned off.
 * `current devider` (**uint**: 1/2/3/4): The output current set by the R_EXT pin can be devided by 1, 2, 3 or 4 if needed. For example, the global output current might be set to 20 mA using R_EXT but some LEDs connected to the IC might only need 10 mA to operate at the desired brightness. In that case, channels that need 20 mA might be configured with a devider of 1 (20 mA / 1 = 20 mA) which is the default. Channels that need 10 mA might be configured with a devider of 2 (20 mA / 2 = 10 mA). Although it is possible to use this to configure the brightness during operation, it is recommended to configure this option once for every output and not use it to change the brightness (use next point instead).
 * `PWM duty cycle` (**uint8**: 0 - 255): The duty cycle can be used to set the brightness of the LED in 256 steps from completely off (0) to completely on (255) and anything in between. This is the recommended way to turn LEDs on and off during normal operation and change their brightness after the initial configuration.