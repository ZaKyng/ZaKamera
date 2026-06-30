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

# Nastavení sériového portu (změňte 'COM17' podle vašeho Arduina)
ser = serial.Serial('COM17', 9600, timeout=1)
blocks = 0 # number of blocks

archive = []

def show_correct(values_list):
    min_value = values_list[0]
    max_value = 0

    max_value = max(values_list)
    min_value = min(values_list)

    region = max_value - min_value
    if region == 0:
        region = 1

    width = 1800 // len(values_list)

    i = 0
    for value in values_list:
        color_value = int((value - min_value) * 255 / region)
        ZaKnode.ColorBlock(parent, pygame.Vector2(width, 720),
            pygame.Color(color_value, color_value, color_value), 
            offset = pygame.Vector2(width * i + 1, 0))
        i += 1

def update_color():
    global blocks, parent, archive
    if ser.in_waiting > 0:
        try:
            if blocks == 0:
                parent.children = []
            # Přečtení řádku, dekódování a převod na celé číslo
            value = ser.readline().decode('utf-8').strip()
            if value == "reset":
                parent.children = []
                archive = []
                blocks = 0
                pass
            if float(value) >= 1.5:
                value = int(float(value) // 1) + 1
            else:
                value = int(float(value) // 1)
            archive.append(value)
            value = min(value // 2, 255)


            ZaKnode.ColorBlock(parent, pygame.Vector2(10, 720),
                pygame.Color(value, value, value), 
                offset = pygame.Vector2(blocks * 10, 0))
            blocks += 1
            if blocks >= 180:
                with open("tests/ad_archive.txt", "a") as f:
                    f.write("picture: " + json.dumps(archive) + "\n\n")
                #print(archive)
                parent.children = []
                show_correct(archive)
                archive = []
                blocks = 0
            
        except ValueError:
            pass



window.run(update_color)

