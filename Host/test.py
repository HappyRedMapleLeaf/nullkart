import serial

# Open serial port
ser = serial.Serial('/dev/ttyACM0', baudrate=9600, timeout=1)

try:
    while True:
        data = ser.read(20)  # Read up to 16 bytes
        if data:
            print(' '.join(f'{b:02X}' for b in data))
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()