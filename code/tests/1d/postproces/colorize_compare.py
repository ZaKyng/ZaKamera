import ast
import pygame
import ZaKnode

pygame.init()
pygame.font.init()

# ----- Pygame setup ----- #

samples = 7

screen_size = (1800, 720)

window = ZaKnode.Game(screen_size, fps = 3)

scenes = []
parents = []

for i in range(samples):
    scenes.append(ZaKnode.Scene("scene_" + str(i), window, (0, 0, 0)))
    parents.append(ZaKnode.BaseNode(scenes[i]))

num_tests = int(input("Number of tested samples: "))

value_lists = []

for i in range(num_tests):
    user_input = input("Give me the list of values: ")
    if user_input.startswith('[') and user_input.endswith(']'):
        value_lists.append(ast.literal_eval(user_input))
    else:
        print("invalid")
        exit()
    
min_value = []
max_value = []

for value_list in value_lists:
    for value in value_list:
        if not isinstance(value, int):
            print("invalid")
            exit()
        
    max_value.append(max(value_list))
    min_value.append(min(value_list))

width = screen_size[0] // len(value_lists[0])
height = screen_size[1] // len(value_lists)
i = 0

def update_color():
    global width, height, value_lists, i
    
    
    if i == 0:
        for x in range(len(value_lists)):
            i = 0
            for value in value_lists[x]:
                color_value = int((value - min_value[x]) * 255 / (max_value[x] - min_value[x]))
                if color_value < 128:
                    main = color_value * 2
                    rest = 0
                else:
                    main = 255
                    rest = (color_value - 128) * 2

                ZaKnode.ColorBlock(parents[0], pygame.Vector2(width, height),
                    pygame.Color(color_value, color_value, color_value), 
                    offset = pygame.Vector2(width * i, height * x))
                
                ZaKnode.ColorBlock(parents[1], pygame.Vector2(width, height),
                    pygame.Color(main, rest, rest), 
                    offset = pygame.Vector2(width * i, height * x))
                
                ZaKnode.ColorBlock(parents[2], pygame.Vector2(width, height),
                    pygame.Color(main, main, rest), 
                    offset = pygame.Vector2(width * i, height * x))
                
                ZaKnode.ColorBlock(parents[3], pygame.Vector2(width, height),
                    pygame.Color(rest, main, main), 
                    offset = pygame.Vector2(width * i, height * x))
                
                ZaKnode.ColorBlock(parents[4], pygame.Vector2(width, height),
                    pygame.Color(rest, main, rest), 
                    offset = pygame.Vector2(width * i, height * x))
                
                ZaKnode.ColorBlock(parents[5], pygame.Vector2(width, height),
                    pygame.Color(rest, rest, main), 
                    offset = pygame.Vector2(width * i, height * x))
                
                ZaKnode.ColorBlock(parents[6], pygame.Vector2(width, height),
                    pygame.Color(main, rest, main), 
                    offset = pygame.Vector2(width * i, height * x))
                i += 1
            

def global_input(event):
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_SPACE:
            window.changeScene()

window.run(update_color, global_input)

