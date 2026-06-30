import ZaKnode
import pygame
import random

window = ZaKnode.Game((900, 900), __file__, "ZaKamera gallery", fps = 30, screen_ratio = 1)
window.fonts.addFont("main", window.directory("assets/font1.ttf"), 4)

def mainColor(value):
    if value < 128:
        return value * 2
    return value

def otherColor(value):
    if value < 128:
        return 0
    return (value - 128) * 2

def HSVtoRGB(h, s, v):
    h = h / 360
    s = s / 100
    v = v / 100

    i = int(h * 6)
    f = h * 6 - i
    p = int(v * 255 * (1 - s))
    q = int(v * 255 * (1 - f * s))
    t = int(v * 255 * (1 - (1 - f) * s))
    v = int(v * 255)

    i = i % 6

    if i == 0:
        return (v, t, p)
    if i == 1:
        return (q, v, p)
    if i == 2:
        return (p, v, t)
    if i == 3:
        return (p, q, v)
    if i == 4:
        return (t, p, v)
    if i == 5:
        return (v, p, q)

modes = [(0, 0), (0, 100), (60, 100), (120, 100), (180, 100), (240, 100), (300, 100), "random"] # (hue, saturation)
modes_names = ["grayscale", "red", "yellow", "green", "cyan", "blue", "magenta", "random"]

lib = ZaKnode.ReadData(window.directory("archive"), "data")

scene_modes = {}

def changeColor(scene, dir):
    global lib, modes, scene_modes

    scene_modes[scene] = (scene_modes[scene] + dir) % len(modes)
    mode = modes[scene_modes[scene]]
    mode_name = modes_names[scene_modes[scene]]

    for block in scene.children[:]:
        if not isinstance(block, ZaKnode.Label):
            block.kill()

    data_set = lib[int(scene.children[0].text) - 1]
    scene.children[1].change(text = mode_name)
    min_value = min(min(line) for line in data_set)
    max_value = max(max(line) for line in data_set)
    #pixel_size = min(900 // len(data_set[0]), 900 // len(data_set))
    pixel_size = (900 // len(data_set[0]), 900 // len(data_set))
    value_range = max(max_value - min_value, 1)

    rand_hue = random.randrange(0, 360)

    for y in range(len(data_set)):
        line = data_set[y]
        for x in range(len(line)):
            value = line[x]
            value = min(int((value - min_value) * 255 / value_range), 255)
            """
            rgb = []
            for i in mode:
                if i == 0:
                    rgb.append(otherColor(value))
                elif i == 1:
                    rgb.append(mainColor(value))
                else:
                    rgb.append(value)
            """

            if mode == "random":
                rgb = HSVtoRGB(rand_hue, 100, value / 255 * 100)
            else:
                rgb = HSVtoRGB(mode[0], mode[1], value / 255 * 100)

            ZaKnode.ColorBlock(scene, pixel_size, rgb, offset = (x * pixel_size[0], y * pixel_size[1]))   

def toggleText(scene):
    for block in scene.children:
        if isinstance(block, ZaKnode.Label):
            block.change(active = block.active == False)

for data_set in lib:
    scene = ZaKnode.Scene("Scene " + str(lib.index(data_set)), window, (0, 0, 0))
    scene_modes[scene] = 0
    ZaKnode.Label(scene, str(lib.index(data_set) + 1), "main", "xl", (220, 0, 0), zindex = 100, offset_str = "top-left", offset = (10, 4))
    ZaKnode.Label(scene, modes_names[0], "main", "s", (0, 180, 180), zindex = 101, offset_str = "bottom-left", offset = (10, -10))
    changeColor(scene, 0)




def changeScene(event):
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_RIGHT:
            window.scenes.changeScene()
        elif event.key == pygame.K_LEFT:
            window.scenes.changeScene(-1)
        elif event.key == pygame.K_UP:
            changeColor(window.scenes.scenes[window.scenes.current_scene], -1)
        elif event.key == pygame.K_DOWN:
            changeColor(window.scenes.scenes[window.scenes.current_scene], 1)
        elif event.key == pygame.K_SPACE:
            toggleText(window.scenes.scenes[window.scenes.current_scene])

window.run(global_input = changeScene)