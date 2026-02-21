# frankentwin
Board layout and Arduino code for replacement Mahlkonig K30 Twin controller

## Features
- Timed output with programming mode to set duration in 0.1s increments for each side
- Continuous mode - Same as Mahlkonig Barista mode, hold in button to grind stop when button is released
- Hold mode - Push button to start grinding, push button again to stop grinding
- Purge mode - Short 1.5s grind to purge grind chamber

## Missing Features
Some features from the original Mahlkonig controller were not implemented on purpose.
- No button LEDs (some of mine were burned out so wasn't worth the time to implement)
- No shot counters (the Arduino EEPROM has a limited number of write cycles)
- No burr replacement interval warning (would need to count shots and it's a pretty rough estimate anyway)

Ideally this replacement board will function even if the low voltage side of the power supply board is damaged.
- No motor current sensing or automatic stall detection

## Usage
### Navigating Modes
- Default mode is timer mode where the current grind duration is shown.
- From timer mode, press single cup button to enter purge mode. Press single cup button again to return to timer mode.
- From timer mode, press double cup button to enter continuous mode. Press double cup button again to toggle between continuous mode and hold mode.
- From continuous/hold mode, press single cup button to return to timer mode.
- From any mode, press programming button to enter programming mode to set timer durations. Single cup button reduces the time by 0.1s, double cup button increases the time by 0.1s. Press programming button again to leave programming mode.
### Grinding
- While grinding, press any of the single cup, double cup, or programming buttons to cancel grinding.
- In continuous mode, hold the grind button to grind. Grinding stops when the button is released.
- In hold mode, press the grind button to start grinding. Press the grind button again to stop grinding.
- In purge mode, press the grind button to start the purge cycle which is a predefined grind duration.
