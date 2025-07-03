import serial
import struct
import time
import math
import matplotlib.pyplot as plt
import matplotlib
import collections

matplotlib.use('TkAgg')

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

# Wait for initial 0xFFFFFFFF
ffCount = 0
while True:
    if ser.in_waiting > 0:
        data = ser.read(1)

        if data == b'\xff':
            ffCount += 1
            if ffCount == 4:
                print("first start pattern encountered")
                break
    else:
        time.sleep(0.05)

# skip first quaternion
while True:
    if ser.in_waiting >= 16:
        data = ser.read(16)
        break
    else:
        time.sleep(0.01)

# Initialize interactive mode
plt.ion()

# Create a plot
fig, ax = plt.subplots()
line_angvels, = ax.plot([], [], label="Angvel", color='r')
line_targets, = ax.plot([], [], label="AngvelTar", color='g')
line_errors, = ax.plot([], [], label="AngvelErr", color='b')
line_angles, = ax.plot([], [], label="Angle", color='y')

ax.legend()

# Set up the plot's axis labels and title
ax.set_xlim(0, 199) # 200 data points
ax.set_ylim(-60, 60)
ax.set_xlabel('time')
ax.set_ylabel('rps')
ax.set_title('motor angular velocity')

exit = False

# Define the close event handler to exit the program
def on_close(event):
    global exit
    plt.close()  # Close the plot
    exit = True

# Connect the close event to the handler
fig.canvas.mpl_connect('close_event', on_close)

angvels = collections.deque(maxlen=200)
targets = collections.deque(maxlen=200)
errors = collections.deque(maxlen=200)
angles = collections.deque(maxlen=200)

def update_data(vel, angle, target):
    angvels.append(vel)
    angles.append(angle)
    targets.append(target)
    errors.append(vel - target)

update_data(0, 0, 0)

# send = struct.pack('<f', 5*28)
# ser.write(send)

while not exit:
    if ser.in_waiting >= 20:
        data = ser.read(20)

        if data[0:4] != b'\xff\xff\xff\xff':
            print("bad start pattern")
            continue

        read_target = struct.unpack('<f', data[4:8])[0]
        read_pos = struct.unpack('<f', data[8:12])[0]
        read_angle = struct.unpack('<f', data[12:16])[0]
        read_zero = struct.unpack('<f', data[16:20])[0]

        print(read_target, read_pos, read_angle, read_zero)

        update_data(read_pos / 28, math.degrees(read_angle), read_target / 28)

        # Update the plot with the last n readings
        line_angvels.set_xdata(range(len(angvels)))
        line_angvels.set_ydata(angvels)
        line_targets.set_xdata(range(len(targets)))
        line_targets.set_ydata(targets)
        line_errors.set_xdata(range(len(errors)))
        line_errors.set_ydata(errors)
        line_angles.set_xdata(range(len(angles)))
        line_angles.set_ydata(angles)

        fig.canvas.draw_idle()  # Efficiently request a redraw
        plt.gcf().canvas.start_event_loop(0.01)  # Allow GUI interactions
    else:
        time.sleep(0.05)

plt.ioff()