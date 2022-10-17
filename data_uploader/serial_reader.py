import serial
import json
import xml.etree.ElementTree as ET

debug = True

if debug:
    # x = input("Please input the port number:")
    str = b'2,3,0,4,2,\r\n'
else:
    car = serial.Serial("COM9")
    print("connected to: " + car.name)


cargo = [0 for i in range(4)]

while True:
    if debug:
        count, cargo[0], cargo[1], cargo[2], cargo[3], additional = str.decode().split(
            ',')
    else:
        count, cargo[0], cargo[1], cargo[2], cargo[3], additional = car.readline(
        ).decode().split(',')

    count = int(count)
    cargo = list(map(int, cargo))
    # raw = {"count": count, "cargo": cargo, "additional": additional}
    # data = json.dumps(raw)

    print(cargo)

car.close()
