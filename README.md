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
PA5 SPI_CLK (short pins 1-2 of P14)
PA6 SPI_MISO
PA7 SPI_MOSI
PB6 SPI_CS for EEPROM
PA8 RESET

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
R PH0, PH1

Servo PWMs (output):
PB3 2/2
PB10 2/3

ADCs: PA4 PB0 PC1 (channels 4, 8, 11)

# CONVENTIONS
CCW is +

```
cd Host
source ../.venv/bin/activate
python3 read_and_graph.py
```