import ast
import pygame
import ZaKnode

pygame.init()
pygame.font.init()

# ----- Pygame setup ----- #

window = ZaKnode.Game((1800, 720), fps = 30)

scene = ZaKnode.Scene("Only_scene", window, (0, 14, 0))

parent = ZaKnode.BaseNode(scene)

input = input("Give me the values_list: ")
if input.startswith('[') and input.endswith(']'):
    values_list = ast.literal_eval(input)
else:
    print("invalid")
    exit()
min_value = values_list[0]
max_value = 0
for value in values_list:
    if not isinstance(value, int):
        print("invalid")
        exit()
    max_value = max(max_value, value)
    min_value = min(min_value, value)

width = 1800 // len(values_list)
i = 0

def update_color():
    global width, values_list, i
    
    
    if i == 0:
        for value in values_list:
            color_value = int((value - min_value) * 255 / (max_value - min_value))
            ZaKnode.ColorBlock(parent, pygame.Vector2(width, 720),
                pygame.Color(color_value, color_value, color_value), 
                offset = pygame.Vector2(width * i + 1, 0))
            i += 1
            



window.run(update_color)

