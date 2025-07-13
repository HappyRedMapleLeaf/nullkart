import asyncio
import struct
from bleak import BleakScanner, BleakClient

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
                    
                    if len(data) != 16:
                        print("Unexpected data length received.")
                        continue
                    
                    # Unpack uint64's from bytes (little-endian format)
                    uint1, uint2 = struct.unpack('<QQ', data)
                    print(f"Read values: {uint1}, {uint2}")
                
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