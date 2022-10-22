import datetime
from random import random
from time import sleep
from xml.etree.ElementInclude import include
import serial
import json
import xml.etree.ElementTree as ET
import random

debug = False

if debug:
    # x = input("Please input the port number:")
    string = b'2,3,0,4,2,\r\n'
    new_xml_name = "cargo"+datetime.datetime.now().strftime("%Y%m%d%H%M%S")+".xml"
else:
    while(True):
        try:
            car = serial.Serial("COM8")
            print("connected to: " + car.name)
            new_xml_name = "receive.xml"
            break
        except:
            print("retrying...")
            sleep(1)
            continue


cargo = [0 for i in range(4)]


init = ET.parse('cargo_empty.xml')
init.write("xmls/"+new_xml_name)


def append_xml(id, goodID, goodName, position, type, Robot_robotID):
    tree = ET.parse("xmls/"+new_xml_name)
    root = tree.getroot()
    cargo = ET.SubElement(root, 'good')
    cargo.set('id', id)
    ET.SubElement(cargo, 'goodID').text = goodID
    ET.SubElement(cargo, 'goodName').text = goodName
    ET.SubElement(cargo, 'position').text = position
    ET.SubElement(cargo, 'type').text = type
    ET.SubElement(cargo, 'robotID').text = Robot_robotID
    tree.write("xmls/"+new_xml_name)


def int_2_alphabet(a):
    b = int(a/26)
    c = a % 26
    return str(chr(b+65))+str(chr(c+65))


good_names = ["food", "medicine", "clothes", "books", "electronics", "other"]
colors = ["red", "green", "blue", "yellow", "empty"]
alphabets = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
             "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"]
letter = alphabets[random.randint(0, 25)]

print("This letter is: "+letter)


try:
    while True:
        # for j in range(4):
        if debug:
            r = string.decode()
        else:
            r = car.readline().decode()

        if r == "Platform Reached\r\n":
            print("Platform Reached")
            continue

        print("count: " + str(cargo[0]), "cargo:"+str(cargo))
        count, cargo[0], cargo[1], cargo[2], cargo[3], additional = r.split(
            ',')

        count = int(count)

        cargo = list(map(int, cargo))
        # raw = {"count": count, "cargo": cargo, "additional": additional}
        # data = json.dumps(raw)
        for i in range(len(cargo)):
            name = str(count*10+i)
            name = letter+name.zfill(4)
            append_xml(str(count*10+i), name, good_names[random.randint(0, 5)],
                       str(i), colors[cargo[i]], "001")

        print(cargo)

except KeyboardInterrupt:
    if not debug:
        car.close()
