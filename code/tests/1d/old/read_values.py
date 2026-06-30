import serial
import json
import pygame
import ZaKnode

pygame.init()
pygame.font.init()

# ----- Pygame setup ----- #

window = ZaKnode.Game((1800, 720), fps = 30)

scene = ZaKnode.Scene("Only_scene", window, (0, 14, 0))

parent = ZaKnode.BaseNode(scene)

# Nastavení sériového portu (změňte 'COM3' podle vašeho Arduina)
ser = serial.Serial('COM17', 9600, timeout=1)
blocks = 0 # number of blocks
max_val = 0

archive = []

def update_color():
    global blocks, parent, max_val, archive
    if ser.in_waiting > 0:
        try:
            # Přečtení řádku, dekódování a převod na celé číslo
            value = ser.readline().decode('utf-8').strip()
            value = int(float(value) // 1)
            archive.append(value)
            value = min(value // 2, 255)


            ZaKnode.ColorBlock(parent, pygame.Vector2(10, 720),
                pygame.Color(value, value, value), 
                offset = pygame.Vector2(blocks * 10, 0))
            blocks += 1
            if blocks >= 180:
                with open("photoresistor-camera/tests/archive.txt", "a") as f:
                    f.write("picture: " + json.dumps(archive) + "\n\n")
                #print(archive)
                archive = []
                parent.children = []
                blocks = 0
            
        except ValueError:
            pass



window.run(update_color)

