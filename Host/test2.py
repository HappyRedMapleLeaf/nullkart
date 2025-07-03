import serial
import struct

# Open serial port
ser = serial.Serial('/dev/ttyACM0', baudrate=9600, timeout=1)

ser.reset_input_buffer()

send = struct.pack('<f', 5)  # rps to tps
ser.write(send)

datacount = 0

while datacount < 20:
    if ser.in_waiting > 0:
        data = ser.read(1)
        datacount += 1
        if data:
            print(' '.join(f'{b:02X}' for b in data))


if ser.in_waiting > 0:
    print("fk")

ser.close()