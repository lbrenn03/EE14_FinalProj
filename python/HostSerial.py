import serial
import time

# ser = serial.Serial(
#     port = 'COM3',
#     baudrate = 115200,
#     timeout = 1
# )

with open('NewCode.bin', 'rb') as file:
        while True:
            chunk = file.read(16)
            if not chunk:
                break

        print(chunk.hex())




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
