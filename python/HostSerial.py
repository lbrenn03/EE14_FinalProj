
import serial
import time
import os

ser = serial.Serial(
    port = 'COM3',
    baudrate = 115200,
    timeout = 30
)
time.sleep(2)

size = os.path.getsize('firmware.bin')
print(f"File size: {size} bytes")

ser.read(size=4)
ser.write(size.to_bytes(4, byteorder='little'))
test = ser.read(size=6)
print(test.decode("utf-8"))

with open('firmware.bin', 'rb') as file:
        while True:
            chunk = file.read(size)
            print(chunk.hex(' '))
            if not chunk:
                break
            ser.write(chunk)


response = ser.read(size=size)
print("Board said:", response.hex(' '))

ser.close()



# import serial
# import time

# ser = serial.Serial(
#     port = 'COM3',
#     baudrate = 115200,
#     timeout = 1
# )

# time.sleep(2)

# ser.write(b'Hello STM32!\r\n')

# response = ser.readline()
# print("Board said:", response.decode('utf-8'))

# ser.close()
