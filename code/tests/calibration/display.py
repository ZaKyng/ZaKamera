import serial
import json
import pygame
import ZaKnode

pygame.init()
pygame.font.init()

# ----- Pygame setup ----- #

window = ZaKnode.Game((900, 900), __file__, fps = 30000, screen_ratio = 1/1)

scene = ZaKnode.Scene("Only_scene", window, (0, 120, 0))

block = ZaKnode.ColorBlock(scene, scene.size, color = (0, 0, 0, 255))


# Nastavení sériového portu (změňte 'COM17' podle vašeho Arduina)
ser = serial.Serial('COM17', 115200, timeout=1)

def update_color():
    if ser.in_waiting > 0:
        try:
            value = ser.readline().decode('utf-8').strip()
            brightness = int(value) 
            brightness = int(brightness * 255/1023)
            print(value, brightness)
            block.change(color = (brightness, brightness, brightness, 255))
        except ValueError:
            pass



window.run(update_color)

