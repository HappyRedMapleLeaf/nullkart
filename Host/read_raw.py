import serial
import time

# Configure the serial port (adjust the port name and settings)
ser = serial.Serial('/dev/ttyACM0', baudrate=9600, timeout=0)
ser.bytesize = 8   # 8 data bits
ser.parity = 'N'   # No parity
ser.stopbits = 1   # 1 stop bit

print("sleeping...")
time.sleep(1)

print("resetting...")
ser.reset_input_buffer()

print("starting read...")
while True:
    if ser.in_waiting > 0:
        data = ser.read(1)
        print(data.hex(), end=' ', flush=True)