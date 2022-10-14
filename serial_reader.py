import serial
import json

# x=input("Please input the port number:")

car = serial.Serial("COM9")

print("connected to: " + car.name)
cargo = [0 for i in range(4)]
# str = b'2,3,0,4,2,\r\n'.decode()

while True:
    count, cargo[0], cargo[1], cargo[2], cargo[3], additional = car.readline(
    ).decode().split(',')
    # count, cargo[0], cargo[1], cargo[2], cargo[3], additional = str.split(',')
    raw = {"count": count, "cargo": cargo, "additional": additional}
    data = json.dumps(raw)
    print(data)

car.close()
