import asyncio
import struct
import matplotlib.pyplot as plt
import numpy as np
from bleak import BleakScanner, BleakClient

READ_MODE_2_UINT64 = 0
READ_MODE_6_FLOAT = 1
read_mode = READ_MODE_6_FLOAT

plt.ion()

async def scan_target_device(target_device, scan_timeout):
    """Scan for BLE devices and return found devices"""
    print("Scanning for BLE devices...")
    devices = {}
    
    def detection_callback(device, advertisement_data):
        devices[device.address] = {
            'device': device,
            'rssi': advertisement_data.rssi,
            'name': device.name or 'Unknown'
        }
        print(f"Found: {device.name or 'Unknown'} ({device.address}) - {advertisement_data.rssi} dBm")
    
    scanner = BleakScanner(detection_callback)
    await scanner.start()
    
    # scan until found or timeout
    for _ in range(int(scan_timeout * 10)):  # 0.1s intervals
        await asyncio.sleep(0.1)
        if target_device in devices:
            print(f"Target device {target_device} found!")
            await scanner.stop()
            return True
    
    await scanner.stop()
    return False

async def connect_to_device(device_address):
    """Connect to a BLE device and return the client"""
    print(f"Attempting to connect to {device_address}...")
    
    try:
        client = BleakClient(device_address)
        await client.connect()
        
        print(f"Connected to {device_address}")
        print(f"Device name: {client.address}")
        print(f"Is connected: {client.is_connected}")
        
        # List available services
        services = client.services
        print("Available services:")
        for service in services:
            print(f"  Service: {service.uuid}")
            for char in service.characteristics:
                print(f"    Characteristic: {char.uuid} (Properties: {char.properties})")

        return client
            
    except Exception as e:
        print(f"Failed to connect to {device_address}: {e}")
        return None

async def run_cli(client, WRITE_UUID, READ_UUID):
    """CLI for entering two floats and writing to characteristic"""    
    try:
        while client.is_connected:
            try:
                user_input = input("\nEnter command (or 'q'): ").strip().lower().split()
                if user_input[0] == 'q':
                    break
                elif user_input[0] == 'h':
                    print("Available commands:")
                    print("  h: Show this help")
                    print("  q: Quit the CLI")
                    print("  w <float1> <float2>: Write two floats to characteristic")
                    print("  r: Read values")
                elif user_input[0] == 'w':
                    if len(user_input) != 3:
                        print("Wrong number of arguments.")
                        continue
                    
                    try:
                        float1 = float(user_input[1])
                        float2 = float(user_input[2])
                    except ValueError:
                        print("Arguments could not be converted to floats.")
                        continue
                    
                    # Pack floats as bytes (little-endian format)
                    data = struct.pack('<ff', float1, float2)
                    
                    print(f"Writing floats {float1}, {float2}")
                    await client.write_gatt_char(WRITE_UUID, data)
                    print("Success")
                elif user_input[0] == 'r':
                    data = await client.read_gatt_char(READ_UUID)
                    
                    if len(data) != 24:
                        print("Unexpected data length received.")
                        continue

                    if read_mode == READ_MODE_2_UINT64:
                        uint1, uint2, _ = struct.unpack('<QQQ', data)
                        print(f"Read values: {uint1}, {uint2}")
                    elif read_mode == READ_MODE_6_FLOAT:
                        values = struct.unpack('<ffffff', data)
                        print(f"Read values: {', '.join(map(str, values))}")

                        x, y, heading = values[0], values[1], values[2]
                        
                        plt.clf()  # Clear current figure instead of creating new one
                        plt.scatter(x, y, s=100, c='blue', marker='o')
                        
                        # Draw heading arrow
                        arrow_length = 0.1
                        dx = arrow_length * np.cos(heading)
                        dy = arrow_length * np.sin(heading)
                        plt.arrow(x, y, dx, dy, head_width=0.02, head_length=0.02, fc='red', ec='red')
                        
                        plt.xlabel('X Position (m)')
                        plt.ylabel('Y Position (m)')
                        plt.title(f'Robot Position: ({x:.3f}, {y:.3f}), Heading: {heading:.3f} rad')
                        plt.grid(True)
                        plt.xlim(-2, 2)
                        plt.ylim(-2, 2)
                        plt.draw()  # Update the plot without showing/focusing
                        plt.pause(0.001)  # Brief pause to allow update
                
            except Exception as e:
                print(f"Error: {e}")
                break
            except KeyboardInterrupt:
                print("\nExiting CLI...")
                break
    
    finally:
        if client.is_connected:
            await client.disconnect()
            print("Disconnected from device")

async def main():
    """Main function that orchestrates scan, connect, and CLI"""
    TARGET_DEVICE = "DC:47:10:B2:48:C4"
    WRITE_UUID = "45447eb2-7bf6-48f1-a4ff-08e14c92224c"
    READ_UUID = "6e18d67b-5fd7-4571-828e-fa04a317f5e3"
    
    # Step 1: Scan for devices
    ret = await scan_target_device(TARGET_DEVICE, 5.0)
    if not ret:
        print(f"Target device not found!")
        return
    
    # Step 2: Connect to device
    client = await connect_to_device(TARGET_DEVICE)
    
    if not client:
        print("Failed to connect or characteristic not found")
        return
    
    # Step 3: Run CLI
    await run_cli(client, WRITE_UUID, READ_UUID)

if __name__ == "__main__":
    asyncio.run(main())