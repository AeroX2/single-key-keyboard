# Single Key Keyboard Setup Guide

Welcome to the Single Key Keyboard setup guide! If you're here, it means you're interested in the single key keyboard, and we'll show you how to customize your message. Follow the instructions below.

## Installation
The only dependency you'll need is the `hid` Python package. You can install it with the following command:

```bash
pip install hid
```

## Usage
Once you've installed the necessary package, you can change the keyboard message and follow these instructions:

```bash
python single_key_keyboard.py
```

# Programming

If you are looking to fully reprogram the board, then read on!

## Pins
- **PIN2:** This pin is connected to an LED. Please note that the LED resistor is not populated. You can use a piece of wire in its place, as the current output by the microcontroller is too low to damage the LED.
- **PIN3:** This pin is for the actual keyboard switch. You can either plug the keyboard switch directly into the Kalih hotswap socket or solder it directly onto the board.
- **PIN4:** There is a switch on the board, but it is not currently in use.

## Compilation
The project is developed in VSCode with the PlatformIO plugin. To get started, simply import the PlatformIO project and click the compile and upload button. It should work seamlessly.

## Programming
To program this board, you'll need an FTDI USB to Serial interface. The board is powered by an ATTINY416 and is programmed through the UPDI interface. There's already a resistor on the board, so you don't need to add another one.

The programming headers can be found on the bottom of the board, and here's a picture of the wiring:

![Programming Wiring](https://github.com/AeroX2/single-key-keyboard/assets/4327898/f5fcdf41-860f-4165-aa51-e61e8e16cc79)

Please note that the TXD and RXD connections might need to be swapped if things don't work as expected.

## Contributing
We welcome pull requests. If you plan to make major changes, please consider opening an issue first to discuss your ideas.

## License
This project is licensed under the [MIT License](https://choosealicense.com/licenses/mit/).
