# TouchGlove

This repository contains software and instructions for creating a homemade dataglove.

![Main setup](./images/small/setup.jpg?raw=true "Main setup")

## Aim
The aim was to create a gesture input device to use for a laptop. One of the main goals was to be able to grab a window/pen
with two fingers and then moving it around.
The hardware was very cheap as it only required a couple of wires, a winter glove and a Dragonboard 410c (DB) - yes, the DB is
a bit more expensive but any other controller like Raspberry Pi or Arduino can also do the job.

## Functionality
The functionality covers:
- mouse movement
- mouse clicks, mouse drag
- changing mouse speed
- switching between the windows

**Mouse movement**:
To move the mouse around simply move your hand around in front of your webcam. Make sure the tracker target is well visible.
You can also cover the tracker target and move your hand, which will have similar effect to lifting and moving your mouse.

**Mouse click, mouse drag**:
Touching you index finger and your thumb will simulate the pressing of the mouse button; putting your fingers apart will
release the mouse button. If you do this quickly it will simulate a mouse click.
You can also move your hand while holding something and thus drag it around the screen.

**Changing mouse speed**:
To change the mouse speed touch your ring finger and your thumb. Touch multiple times to scroll through all the speeds.

**Switching between the windows**:
To enable the window switching mode, touch your middle finger and your thumb. Now you can touch your index finger to go to
the last active window. Or you can touch your ring finger to circulate through all the open windows.
To toggle the window switching mode off, touch your middle finger and your thumb again.

## Components list
The whole setup contains the following components:
- a laptop running Debian (should work for any other Linux distribution as well)
- a webcam (I am using my laptop's built-in camera)
- a Dragonboard 410c running Debian
- a power adapter for Dragonboard 410c 
- a micro USB cable (that can transmit data)
- a glove (I used a woolen winter glove that has been living in our house for several years without an owner)
- wires
- motion tracker target (I used the top of a plastic bottle cap that had a very contrasting green colour)

## Software
On your laptop you will need the following programs:
- NodeJs
- npm
- Python
- xte (part of [xautomation](https://linux.die.net/man/7/xautomation); on Debian install with `sudo apt-get install xautomation`)
- notify-send (on Debian install with `sudo apt-get install libnotify-bin`)
- All the files in the [`laptopApp/` folder](./laptopApp) of this repository

On your DB you will need the following programs:
- g++ compiler
- All the files in the [`Dragonboard/` folder](./Dragonboard) of this repository

## Hardware setup
### Glove sensors
First, we need something to be able to get the input from our fingers to the DB. For this I decided to create the very basic electric
switch which is sewn into the glove. 
There are four fingers that are involved in the actions. Index finger, middle finger and ring finger are the fingers that
actually have actions associated with them. Thumb is the one that can trigger any of the sensors on the other fingers. Therefore,
if the index finger and the middle finger accidentally touch, no action is triggered.
To create the sensors, I simply sew some wire into the woolen glove (see images). I used the same pattern on all three action
fingers. However, the pattern on the thumb is rotated for 90 degrees so that the sensors get triggered more easily.

**Note:** This part can be completed in countless different ways. I just went with this solution as it was good enough for a
prototype as well as being very cheap (aka free :) ).

![The front side of the glove with visible sensors and tracker target.](./images/small/glove_frontside.jpg?raw=true "The front side of the glove with visible sensors and tracker target.")

### Motion tracker target
The program that runs on the laptop is tracking an object that is of a contrasting colour compared to the background. For this
I used a top of a bright green plastic bottle cap, that I attached to the front of the glove (see images).
When testing different colours and objects I realised that a small solid object with a strong colour is very suitable
for the job. Should you use an object that can bend easily you might notice that flickering can increase due to the shadows
which could cause the mouse to jump around the screen.

![Tracker is tracking this green circle to move the mouse.](./images/small/tracker_target.jpg?raw=true "Tracker is tracking this green circle to move the mouse.")

### Connect glove and Dragonboard
Once the sensors are completed, we need to connect them to the GPIO pins on the DB. I connected them with longer wires
to allow for almost unrestricted hand movement.
Now, connect the **wire from the thumb** to **pin 35 (1.8V PWR)**.
Connect the rest of the wires to pins with numbers from **24 to 34** (including). Remember which finger you connected to which
pin as you will need this number later!

![Wires from the four fingers connected to the GPIO pins on the Dragonboard.](./images/small/gpio_connections.jpg?raw=true "Wires from the four fingers connected to the GPIO pins on the Dragonboard.")

### Connect Dragonboard with the laptop
Now connect the power supply to the DB and use micro USB cable to connect your laptop with MicroUSB connector (J4) on the DB.

## Configuration
The hardware is now ready, but before we start the programs we still need to configure them. Changing some of these
parameters can influence the performance, movement smoothness, etc., so feel free to try different values. However, there are
some parameters that need to be set correctly to allow the program to even run at all.

### Laptop application configuration
The current application works with three sensors. If you decide to add/remove sensors or assign different actions
to the sensors you will need to modify the code in `action-manager.js`.

The hand motion tracker is more easily configurable. Navigate to `hand-tracker.py`. The configurable variables are on 
the top of the file:
- `lower` and `upper`: These two values specify the color range for your motion tracker target. The values need to be in 
HSV format.
- `displayTracker`: Set to True if you want to see the video display of the tracker (Useful when debugging or setting the
colour range).
- `frameW`: In pixels. This specifies the size of the frame on which all the calculations will be made. Note that if this
number is to large it may impact the performance.
- `frameH`: You do not need to set this manually as it will be calculated at the start of the program to correspond to your
screen ratio.
- `moveThreshold_lower` and `moveThreshold_upper`: These two values can be configured to ignore certain flickers of the tracker
target.
- `radiusThreshold` and `flickerFramesThreshold`: These values enable the program to ignore sudden flickers in tracker 
target size that usually arise because of the shadows.

### Dragonboard configuration
Navigate to `TouchListener.cpp`. Configurable variables are on the top of the file:
- `threshold_ms`: Time in milliseconds. Tells how long the sensor needs to be pressed for before it triggers an action.
- `numOfSensors`: This tells the program how many sensors we are listening to.
- `sensors`: Array of all Gpio objects. **Here we need to use the same pin numbers we used in step *"Connect 
glove and Dragonboard"***. The first Gpio object needs to correspond to the wire from the index finger; the second Gpio
object needs to correspond to the wire from the middle finger; and the third Gpio object needs to correspond to the
ring finger. All the sensors need to be configured as an "input" pin. 
**Note**: You do not need to configure the pin for the thumb wire!
- `configurationMask`: An integer, for readability written in binary. This mask represents the state of the sensors in which
all the sensors are considered to be "off". Example: if the mask is set to 0b01, this means that the first sensors is triggered
when it is touched; however the second sensor is triggered when it is released (we need to hold it for the sensor to be "off").
- all other control variables need to be initialised with the default values, however they need to be of the **same length as
the sensors array** (this is checked with assertions).

**Important**: the configuration mask bits correspond to the sensors from left to right. This is the leftmost bit corresponds to the
first sensor in the array and the rightmost bit corresponds to the last sensor in the array.

Note: you can use any number of sensors by simply changing how many Gpio object you create. The program will work properly as
long as all other control variables are initialised correctly.

## Software setup
### Laptop
1. Open terminal on your laptop.
2. Navigate to the `laptopApp/` folder that you copied from this repository, for example: `cd ~/Documents/laptopApp/`.
3. Run `npm i` to install the NodeJs modules.

### Dragonboard
I suggest connecting to the DB through an SSH.
1. Open terminal on the DB.
2. Navigate to the `Dragonboard/` folder that you copied from this repository, for example: `cd ~/Documents/Dragonboard/`.
3. Compile the files by executing `make` (If make command doesn't work, install it or just copy the second line 
of the `Makefile` and run it).

The files are now ready to run. However, we also need to switch the DB into a "slave" mode to work with the USB cable.
Note: after this step the keyboard and mouse might stop working (see [this response][0]).

4. In terminal, execute `sudo modprobe g_serial`.

## Running the application
### Laptop
1. Open terminal on your laptop.
2. Navigate to the `laptopApp/` folder that you copied from this repository, for example: `cd ~/Documents/laptopApp/`.
3. Run `npm start`.
4. Provide password, if prompted. The application needs to run as root because of communication via the USB.
5. The application might take a few seconds to finish setting up and starting the webcam.

### Dragonboard
I suggest connecting to the DB through an SSH.
1. Open terminal on the DB.
2. Navigate to the `Dragonboard/` folder that you copied from this repository, for example: `cd ~/Documents/Dragonboard/`.
3. Run `sudo ./TouchListener`.
4. Provide password, if prompted. The application needs to run as root to be able to communicate with GPIOs and to transmit
data via the USB.
5. The setup is completed in less than a second.

*Now, try using the glove you created and moving it around! Have fun!*

![Glove in action.](./images/small/in_front_of_screen.jpg?raw=true "Glove in action.")

## Feedback
Whether you liked the project or not, I will be very thankful for any feedback, suggestions, comments on the project.

*Feel free to [email][1] me!*


[0]: https://discuss.96boards.org/t/serial-read-data-via-dragonboard-usb-port/2880/3
[1]: mailto:luka.kralj2@gmail.com

