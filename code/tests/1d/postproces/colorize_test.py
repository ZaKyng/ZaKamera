import ast
import pygame
import ZaKnode

pygame.init()
pygame.font.init()

# ----- Pygame setup ----- #

samples = 7

screen_size = (1800, 720)

window = ZaKnode.Game(screen_size, fps = 30)

scenes = []
parents = []

for i in range(samples):
    scenes.append(ZaKnode.Scene("scene_" + str(i), window, (0, 0, 0)))
    parents.append(ZaKnode.BaseNode(scenes[i]))

user_input = input("Give me the list of values: ")
if user_input.startswith('[') and user_input.endswith(']'):
    values_list = ast.literal_eval(user_input)
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
            if color_value < 128:
                main = color_value * 2
                rest = 0
            else:
                main = 255
                rest = (color_value - 128) * 2
            ZaKnode.ColorBlock(parents[0], pygame.Vector2(width, screen_size[1]),
                pygame.Color(color_value, color_value, color_value), 
                offset = pygame.Vector2(width * i, 0))
            
            ZaKnode.ColorBlock(parents[1], pygame.Vector2(width, screen_size[1]),
                pygame.Color(main, rest, rest), 
                offset = pygame.Vector2(width * i, 0))
            
            ZaKnode.ColorBlock(parents[2], pygame.Vector2(width, screen_size[1]),
                pygame.Color(main, main, rest), 
                offset = pygame.Vector2(width * i, 0))
            
            ZaKnode.ColorBlock(parents[3], pygame.Vector2(width, screen_size[1]),
                pygame.Color(rest, main, main), 
                offset = pygame.Vector2(width * i, 0))
            
            ZaKnode.ColorBlock(parents[4], pygame.Vector2(width, screen_size[1]),
                pygame.Color(rest, main, rest), 
                offset = pygame.Vector2(width * i, 0))
            
            ZaKnode.ColorBlock(parents[5], pygame.Vector2(width, screen_size[1]),
                pygame.Color(rest, rest, main), 
                offset = pygame.Vector2(width * i, 0))
            
            ZaKnode.ColorBlock(parents[6], pygame.Vector2(width, screen_size[1]),
                pygame.Color(main, rest, main), 
                offset = pygame.Vector2(width * i, 0))
            i += 1
            

def global_input(event):
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_SPACE:
            window.changeScene()

window.run(update_color, global_input)

