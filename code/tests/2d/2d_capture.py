import serial
import json
import pygame
import ZaKnode

pygame.init()
pygame.font.init()

# ----- Pygame setup ----- #

window = ZaKnode.Game((900, 900), __file__, "ZaKamera capture", fps = 30, screen_ratio = 1)

scene = ZaKnode.Scene("Only_scene", window, (0, 14, 0))

parent = ZaKnode.BaseNode(scene)

# Nastavení sériového portu (změňte 'COM17' podle vašeho Arduina)
ser = serial.Serial('COM17', 115200, timeout=1)
blocks = 0 # number of blocks
lines = 0 # number of lines

x_dir = -1

max_val = 0

archive = []
line = []

def show_correct(values_list):
    min_value = values_list[0][0]
    max_value = 0

    for line in values_list:
        max_value = max(max_value, max(line))
        min_value = min(min_value, min(line))

    region = max_value - min_value
    if region <= 0:
        region = 1

    width = 900 // len(values_list[0])
    height = 900 // len(values_list)

    size = min(height, width)

    y = 0

    for line in values_list:
        x = 0
        for value in line:
            color_value = int((value - min_value) * 255 / region)
            ZaKnode.ColorBlock(parent, pygame.Vector2(size, size),
                pygame.Color(color_value, color_value, color_value), 
                offset = pygame.Vector2(size * x, size * y))
            x += 1
        y += 1
    
def update_color():
    global blocks, lines, x_dir, line, parent, max_val, archive
    if ser.in_waiting > 0:
        try:
            if blocks == 0 and lines == 0:
                for child in parent.children:
                    child.kill()
                parent.children.clear()
                archive.clear()
            # Přečtení řádku, dekódování a převod na celé číslo
            value = ser.readline().decode('utf-8').strip()
            if value == "start":
                for child in parent.children:
                    child.kill()
                archive = []
                line.clear()
                blocks = 0
                lines = 0
                x_dir = -1
                return
            elif value == "nl":
                if line != []:
                    archive.append(line.copy())
                    lines += 1
                blocks = 0
                x_dir *= -1
                line.clear()
                return
            elif value == "end":
                lib = ZaKnode.ReadData(window.directory("archive"), "data")
                if lib is None:
                    lib = []
                lib.append(archive)
                ZaKnode.SaveData(window.directory("archive"), "data", lib)

                for child in parent.children:
                    child.kill()
                parent.children.clear()
                blocks = 0
                lines = 0

                #print(len(parent.children))

                show_correct(archive)
                archive.clear()

                
                return

            if float(value) >= 1.5:
                value = int(float(value) // 1) + 1
            else:
                value = int(float(value) // 1)

            color_value = min(value // 2, 255)

            if x_dir == 1:
                line.append(value)
                ZaKnode.ColorBlock(parent, pygame.Vector2(5, 5),
                    pygame.Color(color_value, color_value, color_value), 
                    offset = pygame.Vector2(blocks * 5, lines * 5))
            else:
                line.insert(0, value)
                ZaKnode.ColorBlock(parent, pygame.Vector2(5, 5),
                    pygame.Color(color_value, color_value, color_value), 
                    offset = pygame.Vector2(900 - (blocks + 1) * 5, lines * 5))
    
            blocks += 1
            
        except ValueError:
            pass



window.run(update_color)

