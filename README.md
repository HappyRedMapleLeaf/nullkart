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
top right pins:
PC9 MOTOR1_PWM
PC8 MOTOR1_FWD
PB8 ENC1_A_BLUE
PC6 MOTOR1_REV
PB9 ENC1_B_WHITE

right rail left column 6th from bottom
PB4 I2C3_SDA
8th from bottom
PA8 I2C3_SCL

# CONVENTIONS
CCW is +

```
cd Host
source ../.venv/bin/activate
python3 read_and_graph.py
```