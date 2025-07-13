`screen /dev/ttyACM0 9600`
exit with ctrl+A then K then Y

`screen /dev/ttyACM0 9600 | xxd`

`sudo dmesg | grep tty`
`screen -ls`
`screen -X -S <session_id> quit`

ctrl s to pause ctrl q to continue


`minicom -D /dev/ttyACM0 -b 9600 -H -w`
ctrl a, x, enter

# PIN ASSIGNMENTS
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