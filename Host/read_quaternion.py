import serial
import struct
import time
import matplotlib
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from scipy.spatial.transform import Rotation as R

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

plt.ion()  # Enable interactive mode

# Create the plot once
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
colors = ['r', 'g', 'b']
labels = ['X', 'Y', 'Z']

# Initialize quivers for X, Y, Z axes
quivers = []
for i in range(3):
    qv = ax.quiver(0, 0, 0, 0, 0, 0, color=colors[i], length=1, label=f'{labels[i]}-axis')
    quivers.append(qv)

ax.set_xlim([-1, 1])
ax.set_ylim([-1, 1])
ax.set_zlim([-1, 1])
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.legend()

while True:
    if ser.in_waiting >= 20:
        data = ser.read(20)

        if data[0:4] != b'\xff\xff\xff\xff':
            print("bad start pattern")
            continue
        
        x = struct.unpack('<f', data[4:8])[0]
        y = struct.unpack('<f', data[8:12])[0]
        z = struct.unpack('<f', data[12:16])[0]
        w = struct.unpack('<f', data[16:20])[0]
        q = R.from_quat([y, z, w, x])

        print(x, y, z, w)

        axes = np.eye(3)  # X, Y, Z unit vectors
        rotated_axes = q.apply(axes)

        # Remove old quivers and draw new ones
        for qv in quivers:
            qv.remove()
        quivers = []
        for i in range(3):
            qv = ax.quiver(0, 0, 0,
                           rotated_axes[i, 0], rotated_axes[i, 1], rotated_axes[i, 2],
                           color=colors[i], length=1, label=f'{labels[i]}-axis')
            quivers.append(qv)

        plt.pause(0.01)  # Allow plot to update
        
    else:
        time.sleep(0.05)

plt.ioff()