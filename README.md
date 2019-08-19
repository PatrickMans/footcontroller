# footcontroller

## Overview
A midi footcontroller based on an Arduino Nano v3, especially for use with
a HX stomp. The setup is 4 stomps (buttons) and 4 leds.
To have any idea what is going on, a display is added, but not necessary.
The code is based on the 4-way button sketch by Jeff Saltzman, but extended
for use with 4 buttons. 

## Button assignment:

Preset mode:
1) Preset Down
2) Preset Up
3) Snapshot Up  
4) Tap Tempo

Snapshot mode:
1) Snapshot 1
2) Snapshot 2
3) Snapshot 3  
4) Snapshot 4

Stomp mode:
1)Stomp 4 on/off
2)Stomp 5 on/off
3)Tap Tempo
4)Tuner on/off

Press and hold:
1) Preset Mode
2) Snapshot Mode
3) Stomp Mode
4) Tuner On/off


## Setup: 
> Midi out circuit (TX)
> 4 buttons, active low (D2, D3, D4, D5)
> 4 Leds, active low (D6, D7, D8, D9) for stomps
> 3 Leds, active low (D10, D11, D12) for modes