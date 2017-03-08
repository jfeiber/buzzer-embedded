# Buzzer-Embedded

The embedded code portion of the Buzzer project. For the web app portion/main README see: https://github.com/jfeiber/buzzer. 

## Architecture

The embedded portion of the Buzzer code is writtenly entirely in C++. Two external libraries are used: [SSD1306Ascii](https://github.com/greiman/SSD1306Ascii), a lightweight text-only library for interacting with the OLED screen, and [ArduinoJson](https://github.com/bblanchon/ArduinoJson), a lightweight stack-only library for encoding/decoding JSON. 

## Setup and Deployment

These instructions will get you a copy of the project up and running on your local machine for development purposes. To test you actually need a Buzzer and some cables. 

### Prerequisities

1. [Arduino IDE](https://www.arduino.cc/en/main/software)
2.  Once you have the Arduino IDE installed, install the two required libraries by going to `Sketch`->`Include Libraries`->`Manage Libraries` and search for the two required libraries and install them. [You can alternatively install them manually if that's more your game, although I don't see why you would.](https://www.arduino.cc/en/guide/libraries)

### Installing

1. Clone this repo
2. Open the `buzzer` folder of this repo in the Arduino IDE and compile. Everything should build properly. 

### General Repo Organization
* `/`
  * `/Box CAD Files/`: Contains all the Solidworks part, Solidworks assembly, and .stl files for the Buzzer container.
    * `buzzer_Top.*`: The top part of the Buzzer container.
    * `buzzer_bottom.*`: The bottom, main portion of the Buzzer where most of the components go in.
    * `buzzer.sldasm`: A Solidworks assembly file that shows how the two above parts are supposed to fit together.
  * `/PCB Design Files/`: Contains all the HTML templates used by the web app.
    * `buzzereater.fzz`: A [Fritzing](http://fritzing.org/home/) file for the PCB. Current rev is 2.
    * `/buzzer_gerber/`: Contains the [Gerber](https://en.wikipedia.org/wiki/Gerber_format) files for the Buzzer PCB. This is what's actually sent to the PCB manufacturer. 
  * `/buzzer/`: Contains the actual embedded code files.
  * `readme.md`: The READme you're currently reading.
  
