# Nullkart
A two-wheeled robot that's super simple on the surface, but will help me answer three questions:
1. How do I really make my code "nice"?<br><br>
My internships made me put thought into things I never considered when writing code for projects: API design, code reusability, ease of collaboration... so I wanted to do a project *properly*. No spaghetti and no gibberish.

1. How did my high school robot actually work?<br><br>
I used to compete in the FIRST Tech Challenge where I used FIRST's SDK and a few community-made libraries that let me say "turn 90 degrees" without needing to worry about PWM, pin config, localization algorithms... Now, I want to dig behind the scenes and write EVERY detail myself. I'm allowing only the STM32 HAL and ST's X-CUBE-BLE2 package.

1. What do I even want my next project to be?<br><br>
This one explains itself. I'm out of creative juices and am hoping that this project will give me an idea of which topics I want to explore further.

CAD model: https://cad.onshape.com/documents/3c70c7ce783c477ba1c5844a/w/3e790864695f095d30ca8527/e/a8a76f91ddcaf7e89e2263d5?renderMode=0&uiState=68be1d29abf671b95c9a7cf1

## Some 'note to self's

`screen /dev/ttyACM0 9600`
exit with ctrl+A then K then Y

`screen /dev/ttyACM0 9600 | xxd`

`sudo dmesg | grep tty`
`screen -ls`
`screen -X -S <session_id> quit`

ctrl s to pause ctrl q to continue

`minicom -D /dev/ttyACM0 -b 9600 -H -w`
ctrl a, x, enter

## PIN ASSIGNMENTS
Power: E5V
Ground: everywhere

UART: PA2, PA3

BLE:
PA0 BOOT
PA1 SPI_CS
PB3 SPI_CLK (short pins 2-3 of P14)
PA6 SPI_MISO
PA7 SPI_MOSI
PA8 RESET

Free: PB6, PB3, PB10, PA4, PB0, PC1

RCC OSC: PH0, PH1, PC14, PC15
B1: PC13
LD2: PA5

I2C: PB8, PB9

Left = motor2 on driver, Right = motor1

Motor PWMs (output):
L PB4 3/1
R PC7 3/2

Motor GPIOs (output):
L PB5, PA10
R PC0, PA9

Encoder GPIOs (input):
L PC2, PC3
R PA15, PB7

# CONVENTIONS
CCW is +

```
cd Host
source ../.venv/bin/activate
python3 read_and_graph.py
```
