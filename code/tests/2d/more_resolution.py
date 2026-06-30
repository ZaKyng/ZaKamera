import ZaKnode
import pygame

window = ZaKnode.Game((900, 900), __file__, "ZaKamera gallery", fps = 30, screen_ratio = 1)
window.fonts.addFont("main", window.directory("assets/font1.ttf"), 4)
raw_pictures = ZaKnode.ReadData(window.directory("archive"), "data")

empty_set = []

for x in range(len(raw_pictures)):
    empty_set.append([])

lib = [raw_pictures]

scene_modes = {}


def updateDisplay(scene, data_set):
    for block in scene.children[:]:
        if not isinstance(block, ZaKnode.Label):
            block.kill()
    
    min_value = min(min(line) for line in data_set)
    max_value = max(max(line) for line in data_set)
    #pixel_size = min(900 // len(data_set[0]), 900 // len(data_set))
    pixel_size = (900 / len(data_set[0]), 900 / len(data_set))
    value_range = max(max_value - min_value, 1)

    for y in range(len(data_set)):
        line = data_set[y]
        for x in range(len(line)):
            value = line[x]
            value = min(int((value - min_value) * 255 / value_range), 255)
            ZaKnode.ColorBlock(scene, (pixel_size[0] + 1, pixel_size[1] + 1), (value, value, value), offset = (x * pixel_size[0], y * pixel_size[1]))

def createResolution(data_set):
    new_data_set = []
    for y in range(len(data_set)):
        new_line = []
        line = data_set[y]
        for x in range(len(line)):
            new_line.append(line[x])
            if x < len(line) - 1:
                new_line.append((line[x] + line[x + 1]) // 2)
        new_data_set.append(new_line)
        if y < len(data_set) - 1:
            new_data_set.append([])
    
    for y in range(len(new_data_set)):
        if new_data_set[y] == []:
            line1 = new_data_set[y - 1]
            line2 = new_data_set[y + 1]
            new_line = []
            for x in range(len(line1)):
                if x % 2 == 0:
                    second_pixel = (line1[x] + line2[x]) // 2
                    if x > 0:
                        first_pixel = (line1[x - 1] + line2[x - 1] + new_line[-1] + second_pixel) // 4
                        new_line.append(first_pixel)
                    new_line.append(second_pixel)
            new_data_set[y] = new_line
    
    return new_data_set


def switchResolution(scene, direction):
    global lib, empty_set, scene_modes

    index = int(scene.children[-1].text) - 1

    if max(len(lib[scene_modes[scene]][index][0]), len(lib[scene_modes[scene]][index])) > 140 and direction > 0:
        return

    scene_modes[scene] = max(scene_modes[scene] + direction, 0)
    
    if len(lib) < scene_modes[scene] + 1:
        lib.append(empty_set.copy())

    if lib[scene_modes[scene]][index] == []:
        lib[scene_modes[scene]][index] = createResolution(lib[scene_modes[scene] - 1][index])

    for block in scene.children[:]:
        if not isinstance(block, ZaKnode.Label):
            block.kill()
    
    updateDisplay(scene, lib[scene_modes[scene]][index])






def toggleText(scene):
    for block in scene.children:
        if isinstance(block, ZaKnode.Label):
            block.change(active = block.active == False)

for data_set in lib[0]:
    scene = ZaKnode.Scene("Scene " + str(lib[0].index(data_set)), window, (0, 0, 0))
    scene_modes[scene] = 0
    ZaKnode.Label(scene, str(lib[0].index(data_set) + 1), "main", "xl", (255, 0, 0), zindex = 100, offset_str = "top-left", offset = (10, 4))
    switchResolution(scene, 0)




def changeScene(event):
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_RIGHT:
            window.scenes.changeScene()
        elif event.key == pygame.K_LEFT:
            window.scenes.changeScene(-1)
        elif event.key == pygame.K_UP:
            switchResolution(window.scenes.scenes[window.scenes.current_scene], 1)
        elif event.key == pygame.K_DOWN:
            switchResolution(window.scenes.scenes[window.scenes.current_scene], -1)
        elif event.key == pygame.K_SPACE:
            toggleText(window.scenes.scenes[window.scenes.current_scene])
        elif event.key == pygame.K_a:
            window.scenes.scenes[window.scenes.current_scene].change(offset = (window.scenes.scenes[window.scenes.current_scene].offset[0] - 10, window.scenes.scenes[window.scenes.current_scene].offset[1]))
        elif event.key == pygame.K_d:
            window.scenes.scenes[window.scenes.current_scene].change(offset = (window.scenes.scenes[window.scenes.current_scene].offset[0] + 10, window.scenes.scenes[window.scenes.current_scene].offset[1]))

window.run(global_input = changeScene) 