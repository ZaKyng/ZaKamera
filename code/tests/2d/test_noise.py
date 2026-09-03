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

block = ZaKnode.ColorBlock(parent, pygame.Vector2(900, 900))

# Nastavení sériového portu (změňte 'COM17' podle vašeho Arduina)
ser = serial.Serial('COM17', 115200, timeout=1)
blocks = 0 # number of blocks
lines = 0 # number of lines

max_val = 0
    
def update_color():
    global blocks, lines, x_dir, line, parent, max_val, archive
    if ser.in_waiting > 0:
        try:
            value = ser.readline().decode('utf-8').strip()
            if int(value) > -1:
                max_val = max(max_val, int(value))
                val = int(value) / max_val * 255
                block.change(color = (val, val, val))
            
        except ValueError:
            pass



window.run(update_color)

