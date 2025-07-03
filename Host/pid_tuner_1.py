import serial
import struct
import time
import matplotlib.pyplot as plt
import matplotlib
import collections
import threading

from matplotlib.widgets import TextBox, Slider

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
line_readings, = ax.plot([], [], label="Reading", color='r')
line_targets, = ax.plot([], [], label="Target", color='g')
line_errors, = ax.plot([], [], label="Error", color='b')

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

# Initial variable value
target_rps = 0

ax_slider = plt.axes([0.25, 0.1, 0.65, 0.03])
slider = Slider(ax_slider, 'target rps', -60, 60, valinit=target_rps)

ax_textbox = plt.axes([0.25, 0.2, 0.1, 0.05])
textbox = TextBox(ax_textbox, 'target rps')
textbox.set_val(str(target_rps))

# Function to update both the plot and textbox when slider moves
def update_slider(val):
    global target_rps
    target_rps = slider.val  # Update the variable
    textbox.set_val(f"{target_rps:.2f}")  # Sync textbox with slider

# Function to update both the plot and slider when textbox changes
def update_text(value):
    global target_rps
    try:
        target_rps = float(value)  # Convert input to float
        if -60 <= target_rps <= 60:  # Keep within slider range
            slider.set_val(target_rps)  # Sync slider with textbox
    except ValueError:
        pass  # Ignore invalid input

# Connect the slider and textbox
slider.on_changed(update_slider)
textbox.on_submit(update_text)

readings = collections.deque(maxlen=200)
targets = collections.deque(maxlen=200)
errors = collections.deque(maxlen=200)

def update_data(reading):
    readings.append(reading)
    targets.append(target_rps)
    errors.append(reading - target_rps)

update_data(0)

def send_data():
    send = struct.pack('<f', target_rps*28)  # rps to tps
    ser.write(send)
    if not exit:
        threading.Timer(0.1, send_data).start()  # Schedule next execution

# Start the background task
send_data()

while not exit:
    if ser.in_waiting >= 20:
        data = ser.read(20)

        if data[0:4] != b'\xff\xff\xff\xff':
            print("bad start pattern")
            continue

        read_target = struct.unpack('<f', data[4:8])[0]
        read_pos = struct.unpack('<f', data[8:12])[0]
        read_angle = struct.unpack('<f', data[12:16])[0]
        read_zero2 = struct.unpack('<f', data[16:20])[0]

        print(read_target, read_pos, read_angle, read_zero2)

        value = read_pos
        value /= 28 # 28 ticks per rev
        update_data(value)

        # Update the plot with the last n readings
        line_readings.set_xdata(range(len(readings)))
        line_readings.set_ydata(readings)
        line_targets.set_xdata(range(len(targets)))
        line_targets.set_ydata(targets)
        line_errors.set_xdata(range(len(errors)))
        line_errors.set_ydata(errors)

        fig.canvas.draw_idle()  # Efficiently request a redraw
        plt.gcf().canvas.start_event_loop(0.01)  # Allow GUI interactions
    else:
        time.sleep(0.05)

plt.ioff()